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
#include <ReactorAsterix/cat002/Asterix002Handler.h>

// Library headers
#include <ReactorAsterix/core/AsterixConstants.h>

namespace ReactorAsterix {

using namespace Constants;

/**
 * @brief Constructor for the ASTERIX Category 2 Handler.
 */
Asterix002Handler::Asterix002Handler(std::shared_ptr<SourceStateManager> manager)
    : AsterixCategoryHandler(std::move(manager)) {}

void Asterix002Handler::onAfterDecode(struct timespec ts)
{
    // Retrieve the Time of Day from the decoded report
    uint32_t TOD = report.i002_030.TOD;

    // Timing Synchronization Logic: Triggered by North Marker (Type 1)
    bool isNorth = (report.i002_000.messageType == I002_000_Handler::MESSAGE_TYPE_T::NORTH_MARKER);

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

    // Identify current synchronization status
    report.timeSynchronized = report.sourceRecord->isSynchronized.load(std::memory_order_acquire);

    if (report.timeSynchronized) [[likely]] {
        // Apply the shift
        report.TOD = applyTimeCorrection(TOD, *report.sourceRecord);
    } else {
        // Phase 1: Pre-sync fallback (use raw radar time or arrival time)
        report.TOD = TOD;
        // Still update lastTod for future bit-stitching even if not synced for distribution
        report.sourceRecord->lastTod.store(static_cast<int32_t>(TOD), std::memory_order_relaxed);
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
