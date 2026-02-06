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
#include <ReactorAsterix/cat034/Asterix034Report.h>

// System headers
#include <tuple>
#include <string_view>

// Library headers
#include <ReactorAsterix/core/AsterixDecoderUtils.h>

namespace ReactorAsterix {

bool Asterix034Report::process_all_octets(
        std::string_view fspec, std::string_view& data,
        AsterixStats& /*stats*/)
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
            pair_func(i034_010, i034_010_presence),
            pair_func(i034_000, i034_000_presence),
            pair_func(i034_030, i034_030_presence),
            pair_func(i034_020, i034_020_presence),
            pair_func(i034_041, i034_041_presence),
            pair_func(i034_050, i034_050_presence),
            pair_func(i034_060, i034_060_presence)),
        // Octet 2
        std::make_tuple(
            pair_func(i034_070, i034_070_presence),
            pair_func(i034_100, i034_100_presence),
            pair_func(i034_110, i034_110_presence),
            pair_func(i034_120, i034_120_presence),
            pair_func(i034_090, i034_090_presence))
    );

    if (!decode_fspec_recursive(reader, bit, data, schema)) return false;

    // 010 - Source ID side effect
    if (i034_010_presence) {
        setSourceIdentifier(i034_010.sac, i034_010.sic);
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
