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
#include <ReactorAsterix/cat001/Asterix001Report.h>

// System headers
#include <tuple>
#include <string_view>

// Library headers
#include <ReactorAsterix/core/AsterixDecoderUtils.h>
#include <ReactorAsterix/core/SourceStateManager.h>
#include <ReactorAsterix/cat001/Asterix001Handler.h>

namespace ReactorAsterix {

bool Asterix001Report::process_all_octets(
        std::string_view fspec,
        std::string_view& data,
        AsterixStatsData& stats,
        IAsterixCategoryHandler& parent)
{
    if (!decode_fspec(fspec, data, stats)) {
        return false;
    }

    // 010 - Source ID side effect
    if (i001_010.presence) [[likely]] {
        uint8_t sac = i001_010.sac;
        uint8_t sic = i001_010.sic;

        auto& concreteParent = static_cast<Asterix001Handler&>(parent);

        const SourceRecord* record = concreteParent.getSourceRecordCached(sac, sic);
        setSourceIdentifier(sac, sic, record);
    }

    if (i001_020.typ) {
        stats.uninterpretedItems++;
    }
    if (i001_020.sim) {
        stats.uninterpretedItems++;
    }
    if (i001_020.rab) {
        stats.uninterpretedItems++;
    }
    if (i001_020.tst) {
        stats.uninterpretedItems++;
    }
    if (i001_020.me) {
        stats.uninterpretedItems++;
    }
    if (i001_020.mi) {
        stats.uninterpretedItems++;
    }

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
