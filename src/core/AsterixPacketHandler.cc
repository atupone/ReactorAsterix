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
#include <arpa/inet.h>
#include <cstring>

// Library headers
#include <ReactorAsterix/core/AsterixConstants.h>
#include <ReactorAsterix/core/EndianUtils.h>

namespace ReactorAsterix {

AsterixPacketHandler::~AsterixPacketHandler() {
    forceFlush();
}

/**
 * @brief Registers a handler for a specific ASTERIX category.
 * The handler is linked to the global stats object before ownership is transferred.
 */
void AsterixPacketHandler::registerCategoryHandler(
        std::unique_ptr<IAsterixCategoryHandler> handler) {
    if (!handler) return;

    uint8_t category = handler->Category;

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
void AsterixPacketHandler::handlePacket(const uint8_t data[], size_t size, struct timespec ts) {
    // Fast exit for empty packets
    if (!data || size == 0) [[unlikely]] return;

    // Increment total packets received
    localStats.totalPackets++;

    // Create a view to manage the buffer without manual pointer arithmetic errors
    std::string_view buffer(reinterpret_cast<const char*>(data), size);

    // Continue processing as long as there is enough data for a minimum header + record
    while (buffer.size() >= Constants::MIN_BLOCK_SIZE) {
        size_t blockLength = processDataBlock(buffer, ts);

        if (blockLength > 0) {
            buffer.remove_prefix(blockLength);
        } else [[unlikely]] {
            // Critical parsing error (e.g., bad length), discard remainder of packet
            localStats.malformedBlocks++;
            break;
        }
    }

    // Capture remaining bytes that didn't form a full block
    if (!buffer.empty()) [[unlikely]] {
        localStats.trailingBytesCount++;
    }

    // Periodic Merge
    if (++packetCount >= 1000) {
        stats.merge(localStats);
        localStats.reset();
        packetCount = 0;
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
size_t AsterixPacketHandler::processDataBlock(
        std::string_view block,
        struct timespec ts)
{
    // Bounds check handled by caller (handlePacket), but double check for safety
    if (block.size() < Constants::HEADER_SIZE) [[unlikely]] {
        localStats.malformedBlocks++;
        return 0;
    }

    // Read the ASTERIX Category (CAT) and Length Indicator (LI)
    // from the current block
    const auto category = static_cast<uint8_t>(block[0]);

    // Read Length (Octets 2-3) using helper
    const uint16_t length = readBigEndian<uint16_t>(block.data() + 1);

    // Sanity Checks:
    // Length must be at least the size of the header.
    // Length must not exceed the actual data available in the buffer.
    if (length < Constants::HEADER_SIZE || length > block.size()) [[unlikely]] {
        localStats.malformedBlocks++;
        return 0;
    }
    localStats.totalBlocks++;

    auto* handler = categoryHandlers[category];
    if (!handler) [[unlikely]] {
        localStats.unhandledCategories++;
        return length;
    }

    size_t offset = Constants::HEADER_SIZE;

    // A single Data Block can contain multiple Data Records.
    while (offset < length) {
        // Create a view for the remaining data in this block
        std::string_view remaining = block.substr(offset, length - offset);

        size_t consumed = dispatchRecord(remaining, handler, ts);

        if (consumed > 0) {
            offset += consumed;
        } else [[unlikely]] {
            // If a record cannot be parsed, skip the rest of this block.
            // Track specific record failures.
            localStats.recordParseErrors++;

            // Abort the rest of the block; we cannot trust the stream position.
            break;
        }
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
size_t AsterixPacketHandler::dispatchRecord(
        std::string_view recordView,
        IAsterixCategoryHandler* handler,
        struct timespec ts)
{
    const auto* const data = reinterpret_cast<const uint8_t*>(recordView.data());

    // Calculate F-Spec size
    // The F-Spec is at least 1 byte.
    // If the FX bit (LSB) is set, it extends to the next byte.
    size_t fspecSize;

    // Determine the maximum searchable area for the F-Spec
    const size_t maxSearch = std::min(recordView.size() - 1,
                                      Constants::MAX_FSPEC_SIZE);

    // Calculate F-Spec size safely
    for (fspecSize = 0; fspecSize < maxSearch; ++fspecSize) {
        const uint8_t currentByte = data[fspecSize];

        // Check FX bit: if 0, we found the end
        if (!(currentByte & Constants::FX_BIT)) {
            break;
        }
    }

    // The Sanity Check
    // If fspecSize == maxSearch, we reached the limit without finding FX=0.
    // This means either the F-Spec is too long or it took up the whole record
    // leaving 0 bytes for payload. Both are invalid.
    if (fspecSize == maxSearch) [[unlikely]] {
        return 0;
    }

    // Final Increment
    // Convert the index where we broke into the actual size.
    fspecSize++;

    auto fspec   = recordView.substr(0, fspecSize);
    auto payload = recordView.substr(fspecSize);

    // Handlers should return 0 on failure, not throw exceptions.
    // Polymorphic call into the specific category handler logic.
    size_t consumed = handler->processDataRecord(fspec, payload, ts, localStats);

    if (consumed == 0) [[unlikely]] {
        return 0;
    }

    return fspecSize + consumed;
}

void AsterixPacketHandler::forceFlush() {
    // Check if there is anything 'trapped' in the local counter
    if (packetCount > 0) {
        stats.merge(localStats);
        localStats.reset();
        packetCount = 0;
    }
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
