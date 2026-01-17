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

#pragma once

// System headers
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <string_view>
#include <vector>
#include <atu_reactor/Types.h>

// Library headers
#include <ReactorAsterix/core/IAsterixCategoryHandler.h>
#include <ReactorAsterix/core/AsterixDiagnostics.h>

namespace ReactorAsterix {

/**
 * @class AsterixPacketHandler
 * @brief The central engine for the ReactorAsterix library.
 *
 * This class processes raw binary streams from AtuReactor, identifies
 * ASTERIX data blocks, and dispatches them to registered category handlers.
 */
class alignas(std::hardware_destructive_interference_size) AsterixPacketHandler {
    public:
        AsterixPacketHandler() = default;

        // Delete copy/move to prevent accidental slicing or ownership issues
        // While unique_ptr makes it non-copyable naturally, explicit deletion
        // documents intent for this "Engine" type class.
        AsterixPacketHandler(const AsterixPacketHandler&) = delete;
        AsterixPacketHandler& operator=(const AsterixPacketHandler&) = delete;

        /**
         * @brief The Bridge for AtuReactor.
         * Marked noexcept to ensure C-linkage compatibility and compiler optimization.
         */
        static void onPacket(void* context,
                             const uint8_t* data, size_t len,
                             uint32_t flags,
                             struct timespec ts) noexcept {
            if (flags & atu_reactor::PacketStatus::TRUNCATED)
                return;
            if (auto* instance = static_cast<AsterixPacketHandler*>(context)) {
                instance->handlePacket(data, len, ts);
            }
        }

        /**
         * @brief High-level entry point to process a buffer of ASTERIX data.
         *
         * @param data A pointer to the raw ASTERIX packet data.
         * @param size The total size of the data in bytes.
         */
        void handlePacket(const uint8_t data[], size_t size, struct timespec ts);

        /**
         * @brief Registers a specialized handler for a specific ASTERIX category.
         *
         * @param category The ASTERIX category number.
         * @param handler A pointer to the handler object. The AsterixPacketHandler
         * takes ownership of this pointer.
         */
        void registerCategoryHandler(
                uint8_t category,
                std::unique_ptr<IAsterixCategoryHandler> handler);

        /**
         * @brief Returns a reference to the current diagnostic statistics.
         */
        [[nodiscard]] const AsterixStats& getStats() const { return stats; }

        /**
         * @brief Returns a copyable snapshot of the statistics at this moment.
         * Useful for logging or GUI updates.
         */
        [[nodiscard]] AsterixStatsData getStatsSnapshot() const { return stats.snapshot(); }

    private:
        /**
         * @brief Internal logic to parse the ASTERIX Block header (CAT + LEN).
         *
         * @param dataBlock A pointer to the start of the data block, including the
         * header.
         * @param dataBlockSize The size of the data block in bytes.
         * @return The total length of the processed data block.
         * Returns 0 on error.
         */
        [[nodiscard]] size_t processDataBlock(std::string_view block);

        /**
         * @brief Internal logic to extract F-spec and hand off to the strategy handler.
         *
         * @param recordData A pointer to the start of the data record.
         * @param dataLeft The remaining size of the data block in bytes.
         * @param handler A pointer to the handler for this record's category.
         * @return The total number of bytes consumed by this record,
         * or 0 on error.
         */
        size_t dispatchRecord(std::string_view recordView, IAsterixCategoryHandler* handler);

        // O(1) lookup table for ASTERIX categories (0-255)
        std::array<IAsterixCategoryHandler*, 256> categoryHandlers{};

        // OWNERSHIP: A single vector to own the memory.
        // If categories are added sequentially, they stay contiguous in RAM.
        std::vector<std::unique_ptr<IAsterixCategoryHandler>> categoryPool;

        AsterixStats stats{}; // The stats object is stored here
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
