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
    // 64 bytes is the standard for almost all modern x86 and ARM CPUs
    constexpr std::size_t CacheLineSize = 64;

    /**
     * @brief A POD struct for high-speed local increments and snapshots.
     * This is used for:
     * 1. Thread-local storage to avoid atomic contention.
     * 2. Returning a copyable 'snapshot' of stats to the user.
     */
    struct AsterixStatsData {
        uint64_t totalPackets{0};
        uint64_t totalBlocks{0};
        uint64_t trailingBytesCount{0};
        uint64_t unhandledCategories{0};
        uint64_t malformedBlocks{0};
        uint64_t malformedRecords{0}; 
        uint64_t recordParseErrors{0};
        uint64_t protocolViolations{0};
        uint64_t unhandledItems{0};
        uint64_t uninterpretedItems{0};

        /// Resets all counters to zero.
        void reset() noexcept {
            *this = {};
        }
    };

    /**
     * @struct AsterixStats
     * @brief Thread-safe container using atomics for global statistics counters.
     */
    struct AsterixStats {
        std::atomic<uint64_t> totalPackets{0};
        std::atomic<uint64_t> totalBlocks{0};
        std::atomic<uint64_t> trailingBytesCount{0};
        std::atomic<uint64_t> unhandledCategories{0};
        std::atomic<uint64_t> malformedBlocks{0};
        std::atomic<uint64_t> malformedRecords{0};
        std::atomic<uint64_t> recordParseErrors{0};
        std::atomic<uint64_t> protocolViolations{0};
        std::atomic<uint64_t> unhandledItems{0};
        std::atomic<uint64_t> uninterpretedItems{0};

        /**
         * @brief Merges a local stats object into the global atomics.
         * Use this every N packets to avoid constant cache invalidation.
         */
        void merge(const AsterixStatsData& local) noexcept {
            totalPackets.fetch_add(local.totalPackets, std::memory_order_relaxed);
            totalBlocks.fetch_add(local.totalBlocks, std::memory_order_relaxed);
            trailingBytesCount.fetch_add(local.trailingBytesCount, std::memory_order_relaxed);
            unhandledCategories.fetch_add(local.unhandledCategories, std::memory_order_relaxed);
            malformedBlocks.fetch_add(local.malformedBlocks, std::memory_order_relaxed);
            malformedRecords.fetch_add(local.malformedRecords, std::memory_order_relaxed);
            recordParseErrors.fetch_add(local.recordParseErrors, std::memory_order_relaxed);
            protocolViolations.fetch_add(local.protocolViolations, std::memory_order_relaxed);
            unhandledItems.fetch_add(local.unhandledItems, std::memory_order_relaxed);
            uninterpretedItems.fetch_add(local.uninterpretedItems, std::memory_order_relaxed);
        }

        /**
         * @brief Returns a consistent snapshot of the current global state.
         */
        [[nodiscard]] AsterixStatsData snapshot() const noexcept {
            return {
                totalPackets.load(std::memory_order_relaxed),
                totalBlocks.load(std::memory_order_relaxed),
                trailingBytesCount.load(std::memory_order_relaxed),
                unhandledCategories.load(std::memory_order_relaxed),
                malformedBlocks.load(std::memory_order_relaxed),
                malformedRecords.load(std::memory_order_relaxed),
                recordParseErrors.load(std::memory_order_relaxed),
                protocolViolations.load(std::memory_order_relaxed),
                unhandledItems.load(std::memory_order_relaxed)
            };
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
