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
#include <chrono>
#include <cstdint>
#include <ctime>
#include <ratio>

namespace ReactorAsterix {
    /**
     * @brief Internal utility to handle ASTERIX-specific time conversions.
     * This class and its methods are hidden from the public API.
     */
    class AsterixTime {
        public:
            using AbsoluteTime = std::chrono::system_clock::time_point;

        private:
            // Use standard ratios
            static constexpr uint64_t NS_PER_SEC      = std::nano::den; // 1,000,000,000
            static constexpr uint64_t NS_PER_TICK     = NS_PER_SEC / 128;
            static constexpr int64_t  SECONDS_PER_DAY = 86400LL;

            // Semantic naming for the masking logic
            static constexpr auto HALF_DAY = std::chrono::hours(12);
            static constexpr auto FULL_DAY = std::chrono::hours(24);

        public:
            /**
             * @brief Internal high-performance anchoring logic.
             * Hidden from the user; used by decoders to set AsterixMessage::TOD.
             */
            static AbsoluteTime anchor(uint32_t rawTicks, struct timespec ts) {
                using namespace std::chrono;

                // Find Midnight of the arrival day
                time_t midnightSecs = ts.tv_sec - (ts.tv_sec % SECONDS_PER_DAY);

                // Map ticks to nanoseconds offset from that midnight
                uint64_t todNs = static_cast<uint64_t>(rawTicks) * NS_PER_TICK;

                // Candidate: Midnight + TOD
                auto candidate = system_clock::from_time_t(midnightSecs) + nanoseconds(todNs);

                // Actual Arrival Time
                auto arrival = system_clock::from_time_t(ts.tv_sec) + nanoseconds(ts.tv_nsec);

                // Wrap-around logic using the HALF_DAY threshold
                auto diff = candidate - arrival;

                if (diff > HALF_DAY) [[unlikely]] {
                    candidate -= FULL_DAY; // Actually from yesterday
                } else if (diff < -HALF_DAY) [[unlikely]] {
                    candidate += FULL_DAY; // Actually from tomorrow
                }

                return candidate;
            }
    };

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
