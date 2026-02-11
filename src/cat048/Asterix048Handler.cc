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

namespace ReactorAsterix {

Asterix048Handler::Asterix048Handler(std::shared_ptr<SourceStateManager> manager)
    : AsterixCategoryHandler(std::move(manager)) {}

bool Asterix048Handler::onAfterDecode(struct timespec /*ts*/)
{
    uint32_t TOD = report.i048_140.TOD;

    if (!report.sourceRecord->isSynchronized.load(std::memory_order_acquire)) [[unlikely]] {
        // Still update lastTod for future bit-stitching even if not synced for distribution
        report.sourceRecord->lastTod.store(TOD, std::memory_order_relaxed);
        return false;
    }

    // Apply the shift
    report.TOD = applyTimeCorrection(TOD, *report.sourceRecord);

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
