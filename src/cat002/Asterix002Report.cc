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
        AsterixStats& /* stats*/)
{
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(fspec.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    auto pair_func = [](auto& item, bool& pres) {
        return std::pair<decltype(item)&, bool&>(item, pres);
    };

    // Declarative Schema: tuple to match ASTERIX FSPEC layout
    auto schema = std::make_tuple(
        // Octet 1
        std::make_tuple(
            pair_func(i002_010, i002_010_presence),
            pair_func(i002_000, i002_000_presence),
            pair_func(i002_020, i002_020_presence),
            pair_func(i002_030, i002_030_presence),
            pair_func(i002_041, i002_041_presence),
            pair_func(i002_050, i002_050_presence),
            pair_func(i002_060, i002_060_presence)),
        // Octet 2
        std::make_tuple(
            pair_func(i002_070, i002_070_presence),
            pair_func(i002_100, i002_100_presence),
            pair_func(i002_090, i002_090_presence),
            pair_func(i002_080, i002_080_presence))
    );

    if (!decode_fspec_recursive(reader, bit, data, schema)) return false;

    // 010 - Source ID side effect
    if (i002_010_presence) {
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
