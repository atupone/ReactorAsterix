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

namespace ReactorAsterix {

bool Asterix001Report::process_all_octets(
        std::string_view fspec, std::string_view& data,
        AsterixStatsData& stats)
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
            pair_func(i001_010, i001_010_presence),
            pair_func(i001_020, i001_020_presence),
            pair_func(i001_040, i001_040_presence),
            pair_func(i001_070, i001_070_presence),
            pair_func(i001_090, i001_090_presence),
            pair_func(i001_130, i001_130_presence),
            pair_func(i001_141, i001_141_presence)),
        // Octet 2
        std::make_tuple(
            pair_func(i001_050, i001_050_presence),
            pair_func(i001_120, i001_120_presence),
            pair_func(i001_131, i001_131_presence),
            pair_func(i001_080, i001_080_presence),
            pair_func(i001_100, i001_100_presence),
            pair_func(i001_060, i001_060_presence),
            pair_func(i001_030, i001_030_presence)),
        // Octet 3
        std::make_tuple(
            pair_func(i001_150, i001_150_presence))
    );

    if (!decode_fspec_recursive(reader, bit, data, schema)) return false;

    // 010 - Source ID side effect
    if (i001_010_presence) {
        setSourceIdentifier(i001_010.sac, i001_010.sic);
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
    if (i001_020.extra) {
        stats.uninterpretedItems++;
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
