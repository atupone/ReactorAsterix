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
#include <ReactorAsterix/cat048/Asterix048Handler.h>

// Library headers
#include <ReactorAsterix/core/AsterixTime.h>

namespace ReactorAsterix {

Asterix048Handler::Asterix048Handler(std::shared_ptr<SourceStateManager> manager)
    : AsterixCategoryHandler(std::move(manager)) {}

void Asterix048Handler::onAfterDecode(struct timespec ts)
{
    // Retrieve the Time of Day from the decoded report
    uint32_t TOD = report.i048_140.TOD;

    // Identify sync status from the shared source record
    report.timeSynchronized = report.sourceRecord->isSynchronized.load(std::memory_order_acquire);

    if (report.timeSynchronized) [[likely]] {
        // Apply the shift
        uint32_t correctedTicks = applyTimeCorrection(TOD, *report.sourceRecord);
        // Anchor to absolute time
        report.TOD = AsterixTime::anchor(correctedTicks, ts);
    } else {
        // Anchor the raw TOD
        report.TOD = AsterixTime::anchor(TOD, ts);
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
