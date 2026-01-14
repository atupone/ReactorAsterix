/*
 * Copyright (C) 2026 Alfredo Tupone
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// Interface
#include <ReactorAsterix/core/AsterixPacketHandler.h>

// System headers
#include <algorithm>

// Library headers
#include <ReactorAsterix/core/AsterixConstants.h>

namespace ReactorAsterix {

namespace {
    // Helper to safely read Big Endian 16-bit integer from a view
    // returns 0 if view is too small, but caller usually checks bounds first.
    inline uint16_t readBe16(std::string_view sv, size_t offset) {
        const auto* ptr = reinterpret_cast<const uint8_t*>(sv.data()) + offset;
        return static_cast<uint16_t>((ptr[0] << 8) | ptr[1]);
    }
}

/**
 * @brief Registers a handler for a specific ASTERIX category.
 * The handler is linked to the global stats object before ownership is transferred.
 */
void AsterixPacketHandler::registerCategoryHandler(
        uint8_t category,
        std::unique_ptr<IAsterixCategoryHandler> handler) {
    if (!handler) return;

    // Link the statistics object
    handler->setStats(this->stats);

    // CHECK FOR EXISTING HANDLER (The "Reset" Logic)
    // If the lookup table already has a pointer for this category,
    // we must remove the old owner from the pool.
    if (categoryHandlers[category] != nullptr) {
        auto it = std::remove_if(categoryPool.begin(), categoryPool.end(),
            [target = categoryHandlers[category]](const auto& ptr) {
                return ptr.get() == target;
            });
        categoryPool.erase(it, categoryPool.end());
    }

    // Load the Lookup Table (O(1) access for dispatch)
    categoryHandlers[category] = handler.get();

    // Load the Ownership Pool (Manages memory lifetime)
    categoryPool.push_back(std::move(handler));
}

/**
 * @brief Top-level loop to process a stream of data.
 * ASTERIX packets over UDP often contain multiple concatenated Data Blocks.
 *
 * This function processes a buffer containing one or more concatenated ASTERIX
 * data blocks. Each block is identified by a category and a length field.
 * The function iterates through the buffer, parses each ASTERIX block, and
 * dispatches it to the appropriate handler based on its category.
 *
 * @param data A pointer to the raw ASTERIX frame data.
 * @param size The total length of the ASTERIX frame data in bytes.
 */
void AsterixPacketHandler::handlePacket(const uint8_t data[], size_t size) {
    // Fast exit for empty packets
    if (!data || size == 0) [[unlikely]] return;

    // Increment total packets received
    stats.totalPackets.fetch_add(1, std::memory_order_relaxed);

    // Create a view to manage the buffer without manual pointer arithmetic errors
    std::string_view buffer(reinterpret_cast<const char*>(data), size);

    // Continue processing as long as there is enough data for a minimum header + record
    while (buffer.size() >= Constants::MIN_BLOCK_SIZE) {
        size_t blockLength = processDataBlock(buffer);

        if (blockLength > 0) {
            buffer.remove_prefix(blockLength);
        } else [[unlikely]] {
            // Critical parsing error (e.g., bad length), discard remainder of packet
            stats.malformedBlocks.fetch_add(1, std::memory_order_relaxed);
            break;
        }
    }

    // Capture remaining bytes that didn't form a full block
    if (!buffer.empty()) [[unlikely]] {
        stats.trailingBytesCount.fetch_add(buffer.size(), std::memory_order_relaxed);
    }
}

/**
 * @brief Decodes the ASTERIX Block Header (CAT + LEN) and dispatches to a handler.
 *
 * This method reads the category and length indicator from the block, then
 * iterates through the data records and dispatches them to the `processDataRecord`
 * method for further parsing.
 *
 * @param dataBlock A pointer to the start of the data block, including the
 * header.
 * @param dataBlockSize The size of the data block in bytes.
 * @return The total length of the processed data block. Returns 0 on error.
 */
size_t AsterixPacketHandler::processDataBlock(std::string_view block) {
    // Bounds check handled by caller (handlePacket), but double check for safety
    if (block.size() < Constants::HEADER_SIZE) [[unlikely]] return 0;

    // Read the ASTERIX Category (CAT) and Length Indicator (LI)
    // from the current block
    const auto category = static_cast<uint8_t>(block[0]);

    // Read Length (Octets 2-3) using helper
    const uint16_t length = readBe16(block, 1);

    // Sanity Checks:
    // Length must be at least the size of the header.
    // Length must not exceed the actual data available in the buffer.
    if (length < Constants::HEADER_SIZE || length > block.size()) [[unlikely]] {
        return 0;
    }

    auto* handler = categoryHandlers[category];

    if (handler) [[likely]] {
        size_t offset = Constants::HEADER_SIZE;

        // A single Data Block can contain multiple Data Records.
        while (offset < length) {
            // Create a view for the remaining data in this block
            std::string_view remaining = block.substr(offset, length - offset);

            size_t consumed = dispatchRecord(remaining, handler);

            if (consumed > 0) {
                offset += consumed;
            } else [[unlikely]] {
                // If a record cannot be parsed, skip the rest of this block.
                // Track specific record failures.
                stats.recordParseErrors.fetch_add(1, std::memory_order_relaxed);

                // Abort the rest of the block; we cannot trust the stream position.
                break;
            }
        }
    } else [[unlikely]] {
        // Increment stats if the category is not registered
        stats.unhandledCategories.fetch_add(1, std::memory_order_relaxed);
    }

    // Return the total length of the data block so handlePacket can advance
    // the pointer.
    return length;
}

/**
 * @brief Handles F-Spec extraction and passes the record to the category logic.
 *
 * This method is responsible for determining the length of the Field
 * Specification (F-spec) and separating it from the subsequent data item,
 * then calling the category-specific handler.
 *
 * @param recordData A pointer to the start of the data record.
 * @param dataLeft The remaining size of the data block in bytes.
 * @param handler  The Asterix Category Handler for the specific category
 * @return The total number of bytes consumed by this record, or 0 on error.
 */
size_t AsterixPacketHandler::dispatchRecord(std::string_view recordView, IAsterixCategoryHandler* handler) {
    const auto* const data = reinterpret_cast<const uint8_t*>(recordView.data());

    // Calculate F-Spec size
    // The F-Spec is at least 1 byte.
    // If the FX bit (LSB) is set, it extends to the next byte.
    size_t fspecSize = 0;
    size_t lastDataIdx = 0;
    uint8_t lastDataValue = 0;

    // Calculate F-Spec size safely
    while (true) {
        // Ensure we don't read past the buffer
        if (fspecSize >= recordView.size() || fspecSize >= Constants::MAX_FSPEC_SIZE) [[unlikely]] {
            return 0;
        }

        const uint8_t currentByte = data[fspecSize];

        // If there are bits other than the FX bit (bit 0), save it
        if (currentByte > 1) {
            lastDataIdx = fspecSize;
            lastDataValue = currentByte;
        }

        fspecSize++;

        // Check FX bit
        if (!(currentByte & Constants::FX_BIT)) break;
    }

    // --- CONSOLIDATED BOUNDS CHECK ---
    // Validate if the furthest data bit exceeds MAX_FRNS (128)
    if (lastDataValue > 0) {
        // 128 FRNs = 18 full bytes (126 bits) + 2 bits in the 19th byte
        if (lastDataIdx > 18) [[unlikely]] {
            return 0;
        }
        // If at index 18 (19th byte), only bits 7 and 6 (FRNs 127, 128) are allowed
        if (lastDataIdx == 18 && (lastDataValue & 0x3E)) [[unlikely]] {
            return 0;
        }
    }

    // Ensure we have at least 1 byte of payload (usually) or that the view is valid
    if (fspecSize > recordView.size()) [[unlikely]] return 0;

    auto fspec   = recordView.substr(0, fspecSize);
    auto payload = recordView.substr(fspecSize);

    // Handlers should return 0 on failure, not throw exceptions.
    // Polymorphic call into the specific category handler logic.
    size_t consumed = handler->processDataRecord(fspec, payload);

    if (consumed > 0) [[likely]] {
        return fspecSize + consumed;
    }

    return 0;
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
