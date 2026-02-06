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

void Asterix048Handler::setStats(AsterixStats& s) {
    stats_ptr = &s;
}

bool Asterix048Handler::onAfterDecode(Asterix048Report& report, struct timespec /*ts*/)
{
    uint32_t TOD = report.i048_140.TOD;

    // ALWAYS update the Radar's 24h clock state (for bit-stitching ref)
    report.sourceRecord->lastTod = TOD;

    if (!report.sourceRecord->isSynchronized.load(std::memory_order_acquire)) [[unlikely]] {
        return false;
    }

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
