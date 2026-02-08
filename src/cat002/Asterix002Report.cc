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
#include <ReactorAsterix/cat002/Asterix002Report.h>

// System headers
#include <tuple>
#include <string_view>

// Library headers
#include <ReactorAsterix/core/AsterixDecoderUtils.h>

namespace ReactorAsterix {

bool Asterix002Report::process_all_octets(
        std::string_view fspec, std::string_view& data,
        AsterixStatsData& /* stats*/)
{
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(fspec.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    // Declarative Schema: tuple to match ASTERIX FSPEC layout
    auto schema = std::make_tuple(
        // Octet 1
        std::tie(
            i002_010,
            i002_000,
            i002_020,
            i002_030,
            i002_041,
            i002_050,
            i002_060),
        // Octet 2
        std::tie(
            i002_070,
            i002_100,
            i002_090,
            i002_080)
    );

    if (!decode_fspec_recursive(reader, bit, data, schema)) return false;

    // 010 - Source ID side effect
    if (i002_010.presence) {
        setSourceIdentifier(i002_010.sac, i002_010.sic);
    }

    return true;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
