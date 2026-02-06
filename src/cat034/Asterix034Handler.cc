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

// Own header
#include <ReactorAsterix/cat034/Asterix034Handler.h>

// Library headers
#include <ReactorAsterix/core/AsterixConstants.h>

namespace ReactorAsterix {

using namespace Constants;

/**
 * @brief Constructor for the ASTERIX Category 034 Handler.
 */
Asterix034Handler::Asterix034Handler(std::shared_ptr<SourceStateManager> manager)
    : AsterixCategoryHandler(std::move(manager)) {}

void Asterix034Handler::setStats(AsterixStats& s) {
    stats_ptr = &s;
}

bool Asterix034Handler::onAfterDecode(Asterix034Report& report, struct timespec ts)
{
    uint32_t TOD = report.i034_030.TOD;

    // ALWAYS update the Radar's 24h clock state (for bit-stitching ref)
    report.sourceRecord->lastTod = TOD;

    bool isNorth = (report.i034_000.messageType == I034_000_Handler::MESSAGE_TYPE_T::NORTH_MARKER);
    // Timing Synchronization Logic: Only triggered by North Marker (Type 1)
    if (isNorth) {

        // Get seconds since midnight using simple modulo
        // 86400 seconds in a day
        uint32_t seconds_since_midnight = static_cast<uint32_t>(ts.tv_sec % 86400);

        // Conversion using the exact integer divisor
        uint32_t kernel_128th = (seconds_since_midnight * AST_TOD_UNITS_PER_SEC) +
            static_cast<uint32_t>(static_cast<uint64_t>(ts.tv_nsec) / NS_PER_AST_TOD_UNIT);

        // Calculate difference (Radar - Kernel)
        int32_t diff = static_cast<int32_t>(TOD) - static_cast<int32_t>(kernel_128th);

        // Handle Midnight Wrap using constexpr
        if (diff > static_cast<int32_t>(AST_TOD_HALFDAY_UNITS)) {
            diff -= static_cast<int32_t>(AST_TOD_UNITS_PER_DAY);
        } else if (diff < -static_cast<int32_t>(AST_TOD_HALFDAY_UNITS)) {
            diff += static_cast<int32_t>(AST_TOD_UNITS_PER_DAY);
        }

        // Update both reference time and the moving average offset
        sourceStateManager->updateTimeOffset(report.sourceIdentifier, diff);
    }

    if (!(isNorth || report.sourceRecord->isSynchronized.load(std::memory_order_acquire))) [[unlikely]] {
        return false;
    };

    // APPLY OFFSET FOR LISTENERS (Transition to System Domain)
    // Now we shift the report's TOD to match our local Linux clock
    int32_t corrected = static_cast<int32_t>(TOD) -
        report.sourceRecord->averageOffset;

    // Handle the midnight wrap-around
    constexpr int32_t TICKS_PER_DAY = 86400 * 128;

    if (corrected < 0) {
        corrected += TICKS_PER_DAY; // Handles "just after midnight"
    } else if (corrected >= TICKS_PER_DAY) {
        corrected -= TICKS_PER_DAY; // Handles "just before midnight"
    }

    // Apply the shift
    report.TOD = static_cast<uint32_t>(corrected);

    return true;
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
