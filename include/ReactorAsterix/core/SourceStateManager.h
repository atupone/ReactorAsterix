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
#include <deque>
#include <mutex>
#include <optional>
#include <shared_mutex>

// Library headers
#include <ReactorAsterix/core/SourceIdentifier.h>

namespace ReactorAsterix {

struct alignas(64) SourceRecord {
    // 2. Temporal State (4 bytes)
    mutable std::atomic<int32_t> lastTod{-1};

    // Temporal offset stats (integrated for cache locality)
    std::atomic<int32_t>  averageOffset{0};
    std::atomic<uint32_t> offsetCount{0};

    explicit SourceRecord(SourceIdentifier identifier) : id(identifier) {}

    // Identification (2 bytes)
    SourceIdentifier id;

    // Flag to indicate we have performed at least one valid time sync
    std::atomic<bool> isSynchronized{false};

    // Automatically calculate remaining space
    // Padding ensures that two threads updating different sensors
    // don't conflict on the same CPU cache line (False Sharing).
    char padding[64 - (sizeof(SourceIdentifier) +
            sizeof(std::atomic<int32_t>) * 2 +
            sizeof(std::atomic<uint32_t>) +
            sizeof(std::atomic<bool>))];
};

static_assert(sizeof(SourceRecord) == 64,
        "SourceRecord size must be exactly 64 bytes to prevent false sharing.");

/**
 * @class SourceStateManager
 * @brief Thread-safe manager for tracking radar source state and time offsets.
 */
class SourceStateManager {
    public:
        SourceStateManager() = default;

        /**
         * @brief Zero-overhead lookup for the hot path.
         * Assumes the record likely exists to avoid mutex acquisition.
         */
        SourceRecord* getRecordUnsafe(SourceIdentifier id) const {
            // No lock! Just a linear scan.
            for (auto const& record : sources_) {
                if (record.id == id) return const_cast<SourceRecord*>(&record);
            }
            return nullptr;
        }

        /**
         * @brief High-performance lookup.
         * Uses linear scan which is faster than map lookup for < 100-200 entries.
         */
        SourceRecord* getOrCreateRecord(SourceIdentifier id) {
            // Try to find with a Shared Lock (Multiple threads can read)
            {
                std::shared_lock lock(mutex_);
                for (auto& record : sources_) {
                    if (record.id == id) return &record;
                }
            }

            // Not found? Upgrade to Unique Lock to add a new entry
            std::unique_lock lock(mutex_);

            // Double-check pattern: another thread might have added it
            // while we were switching locks.
            for (auto& record : sources_) {
                if (record.id == id) return &record;
            }

            // Add new record with default stats and padding
            sources_.emplace_back(id);
            return &sources_.back();
        }

        /**
         * @brief Updates the moving average offset for a source.
         * @param si The source identifier.
         * @param diff128th The difference (Radar TOD - Kernel Time) in 1/128s units.
         */
        inline void updateTimeOffset(const SourceIdentifier& si, int32_t diff128th) {
            // Try the lock-free bypass first
            SourceRecord* record = getRecordUnsafe(si);

            // Slow path: Only lock if the sensor is brand new (rare)
            if (!record) [[unlikely]] {
                record = getOrCreateRecord(si);
            }

            // 2. Lock-Free Update
            // We fetch the current count to use in the EMA calculation
            auto count = record->offsetCount.load(std::memory_order_relaxed);

            // Use a window size (e.g., 128) to remain responsive to clock drift.
            // Once the window is full, the average behaves like an
            // Exponential Moving Average (EMA).
            constexpr uint64_t MAX_WINDOW = 128;

            if (count < MAX_WINDOW) {
                record->offsetCount.fetch_add(1, std::memory_order_relaxed);
                count++;
            }

            // Atomic Compare-And-Swap (CAS) loop for the moving average
            int32_t currentAvg = record->averageOffset.load(std::memory_order_relaxed);
            int32_t newAvg;
            do {
                newAvg = currentAvg + (diff128th - currentAvg) / static_cast<int32_t>(count);
            } while (!record->averageOffset.compare_exchange_weak(
                        currentAvg, newAvg,
                        std::memory_order_release,
                        std::memory_order_relaxed));

            // Once we have an offset, we consider this source "Synchronized"
            // Use memory_order_release to ensure the averageOffset is visible to other threads
            if (!record->isSynchronized.load(std::memory_order_relaxed)) [[unlikely]] {
                record->isSynchronized.store(true, std::memory_order_release);
            }
        }

    private:
        // Contiguous storage for cache-friendly lookups
        std::deque<SourceRecord> sources_;
        mutable std::shared_mutex mutex_; // Protects all internal maps
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
