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
#include <map>
#include <cstdint>
#include <optional>
#include <shared_mutex>

// Library headers
#include <ReactorAsterix/core/SourceIdentifier.h>

namespace ReactorAsterix {

/**
 * @class SourceStateManager
 * @brief Thread-safe manager for tracking radar source state and time offsets.
 */
class SourceStateManager {
    public:
        SourceStateManager() = default;

        /**
         * @brief Returns the last known 32-bit TOD for a source.
         */
        [[nodiscard]] std::optional<uint32_t> getReferenceTime(const SourceIdentifier& si) const {
            std::shared_lock lock(mutex_); // Thread-safe read
            if (const auto it = sources.find(si); it != sources.end()) {
                return it->second;
            }
            return std::nullopt;
        }

        /**
         * @brief Updates the stored 32-bit TOD for a specific source.
         * Can be called by CAT 002, 048, 062, etc., whenever a full TOD is available.
         */
        void updateSourceTime(const SourceIdentifier& si, uint32_t fullTod) {
            std::unique_lock lock(mutex_); // Thread-safe write
            sources.insert_or_assign(si, fullTod);
        }

        /**
         * @brief Updates the moving average offset for a source.
         * @param si The source identifier.
         * @param diff128th The difference (Radar TOD - Kernel Time) in 1/128s units.
         */
        void updateTimeOffset(const SourceIdentifier& si, int32_t diff128th) {
            std::unique_lock lock(mutex_);
            auto& data = offsetData[si];

            // Use a window size (e.g., 128) to remain responsive to clock drift.
            // Once the window is full, the average behaves like an
            // Exponential Moving Average (EMA).
            constexpr uint64_t MAX_WINDOW = 128;

            if (data.count < MAX_WINDOW) {
                data.count++;
            }

            // Update average: NewAvg = OldAvg + (NewValue - OldAvg) / N
            data.average += (static_cast<double>(diff128th) - data.average) / static_cast<double>(data.count);
        }

        /**
         * @brief Gets the current average offset in 1/128s units.
         * Returns 0 if no offset has been calculated yet.
         */
        [[nodiscard]] int32_t getAverageOffset(const SourceIdentifier& si) const {
            std::shared_lock lock(mutex_);
            if (auto it = offsetData.find(si); it != offsetData.end()) {
                return static_cast<int32_t>(it->second.average);
            }
            return 0;
        }

    private:
        struct OffsetStats {
            double average = 0.0;
            uint64_t count = 0;
        };

        mutable std::shared_mutex mutex_; // Protects all internal maps

        std::map<SourceIdentifier, uint32_t> sources;

        std::map<SourceIdentifier, OffsetStats> offsetData;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
