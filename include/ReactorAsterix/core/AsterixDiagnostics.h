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
#include <atomic>
#include <cstdint>

namespace ReactorAsterix {

    /**
     * @brief A Plain Old Data (POD) struct representing a snapshot of statistics.
     * Unlike AsterixStats, this CAN be copied, printed, or serialized.
     */
    struct AsterixStatsData {
        uint64_t totalPackets{0};
        uint64_t trailingBytesCount{0};
        uint64_t unhandledCategories{0};
        uint64_t malformedBlocks{0};
        uint64_t malformedRecords{0}; 
        uint64_t recordParseErrors{0};
        uint64_t protocolViolations{0};
        uint64_t unhandledItems{0};
    };

    /**
     * @brief Thread-safe statistics counters.
     * Note: std::atomic is non-copyable.
     */
    struct AsterixStats {
        std::atomic<uint64_t> totalPackets{0};
        std::atomic<uint64_t> trailingBytesCount{0};

        std::atomic<uint64_t> unhandledCategories{0};

        std::atomic<uint64_t> malformedBlocks{0};
        std::atomic<uint64_t> malformedRecords{0};
        std::atomic<uint64_t> recordParseErrors{0};
        std::atomic<uint64_t> protocolViolations{0};
        std::atomic<uint64_t> unhandledItems{0};

        /**
         * @brief Create a copyable snapshot of the current counters.
         * Uses memory_order_relaxed for performance, as strict ordering
         * is rarely required for analytics counters.
         */
        [[nodiscard]] AsterixStatsData snapshot() const noexcept {
            return {
                totalPackets.load(std::memory_order_relaxed),
                trailingBytesCount.load(std::memory_order_relaxed),
                unhandledCategories.load(std::memory_order_relaxed),
                malformedBlocks.load(std::memory_order_relaxed),
                malformedRecords.load(std::memory_order_relaxed),
                recordParseErrors.load(std::memory_order_relaxed),
                protocolViolations.load(std::memory_order_relaxed),
                unhandledItems.load(std::memory_order_relaxed)
            };
        }

        /**
         * @brief Resets all counters to zero.
         */
        void reset() noexcept {
            totalPackets.store(0, std::memory_order_relaxed);
            trailingBytesCount.store(0, std::memory_order_relaxed);
            unhandledCategories.store(0, std::memory_order_relaxed);
            malformedBlocks.store(0, std::memory_order_relaxed);
            malformedRecords.store(0, std::memory_order_relaxed);
            recordParseErrors.store(0, std::memory_order_relaxed);
            protocolViolations.store(0, std::memory_order_relaxed);
            unhandledItems.store(0, std::memory_order_relaxed);
        }
    };
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
