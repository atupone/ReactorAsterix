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
#include <ReactorAsterix/cat048/Asterix048Report.h>

// System headers
#include <tuple>
#include <string_view>

// Library headers
#include <ReactorAsterix/core/AsterixDecoderUtils.h>

namespace ReactorAsterix {

bool Asterix048Report::process_all_octets(
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
            pair_func(i048_010, i048_010_presence),
            pair_func(i048_140, i048_140_presence),
            pair_func(i048_020, i048_020_presence),
            pair_func(i048_040, i048_040_presence),
            pair_func(i048_070, i048_070_presence),
            pair_func(i048_090, i048_090_presence),
            pair_func(i048_130, i048_130_presence)),
        // Octet 2
        std::make_tuple(
            pair_func(i048_220, i048_220_presence),
            pair_func(i048_240, i048_240_presence),
            pair_func(i048_250, i048_250_presence),
            pair_func(i048_161, i048_161_presence),
            pair_func(i048_042, i048_042_presence),
            pair_func(i048_200, i048_200_presence),
            pair_func(i048_170, i048_170_presence)),
        // Octet 3
        std::make_tuple(
            pair_func(i048_210, i048_210_presence),
            pair_func(i048_030, i048_030_presence),
            pair_func(i048_080, i048_080_presence),
            pair_func(i048_100, i048_100_presence),
            pair_func(i048_110, i048_110_presence),
            pair_func(i048_120, i048_120_presence),
            pair_func(i048_230, i048_230_presence)),
        // Octet 4
        std::make_tuple(
            pair_func(i048_260, i048_260_presence),
            pair_func(i048_055, i048_055_presence),
            pair_func(i048_050, i048_050_presence),
            pair_func(i048_065, i048_065_presence),
            pair_func(i048_060, i048_060_presence))
    );

    if (!decode_fspec_recursive(reader, bit, data, schema)) return false;

    // 010 - Source ID side effect
    if (i048_010_presence) {
        setSourceIdentifier(i048_010.sac, i048_010.sic);
    }

    // 020 - Abnormal checks
    if (i048_020_presence && (i048_020.sim || i048_020.rab || i048_020.tst)) {
        stats.uninterpretedItems++;
        return false;
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
