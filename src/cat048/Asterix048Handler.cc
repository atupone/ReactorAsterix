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
    // 2. Propagate the reference to every handler in your compile-time tuple
    std::apply([&s](auto&&... handler) {
        (handler.setStats(s), ...);
    }, m_handlers);
    stats_ptr = &s;
}

bool Asterix048Handler::dispatch(int frn, Asterix048Report& report, std::string_view& data) {
    // Calls the template version in the base class
    return AsterixCategoryHandler::dispatch(frn, report, data, m_handlers);
}

bool Asterix048Handler::onAfterDecode(Asterix048Report& report, struct timespec /*ts*/)
{
    if (!report.sourceRecord->isSynchronized.load(std::memory_order_acquire)) [[unlikely]] {
        return false;
    }

    uint32_t TOD = report.TOD;

    // UPDATE MANAGER FIRST (Using raw Radar TOD)
    // ALWAYS update the Radar's 24h clock state (for bit-stitching ref)
    // Update persistent raw time
    report.sourceRecord->lastTod = TOD;

    // APPLY OFFSET FOR LISTENERS (Transition to System Domain)
    // Now we shift the report's TOD to match our local Linux clock
    // Get the average offset directly from the record (No lookup needed!)
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
