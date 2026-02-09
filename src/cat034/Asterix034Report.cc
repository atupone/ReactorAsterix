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

auto Asterix034Report::get_schema() {
    return std::make_tuple(
        std::tie(i034_010, i034_000, i034_030, i034_020, i034_041, i034_050, i034_060),
        std::tie(i034_070, i034_100, i034_110, i034_120, i034_090, dummy,    dummy)
    );
}

Asterix034Report::Asterix034Report() {
    if (!initialized) {
        auto schema = get_schema();
        min_fspec_len = get_min_fspec_length(schema);
        initialized = true;
    }
}

bool Asterix034Report::process_all_octets(
        std::string_view fspec, std::string_view& data,
        AsterixStatsData& stats)
{
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(fspec.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    // Declarative Schema: tuple to match ASTERIX FSPEC layout
    auto schema = get_schema();

    if (!is_fspec_complete(fspec, min_fspec_len)) {
        stats.protocolViolations++;
    }

    // Decode using schema
    if (!decode_fspec_recursive(reader, bit, data, stats, schema)) {
        return false;
    }

    // 010 - Source ID side effect
    if (i034_010.presence) {
        setSourceIdentifier(i034_010.sac, i034_010.sic);
    }

    return true;
}

void Asterix034Report::reset() {
    // Reset common base fields (SourceIdentifier, TOD, etc.)
    AsterixReport::reset();

    i034_010.reset();
    i034_000.reset();
    i034_030.reset();
    i034_020.reset();
    i034_041.reset();
    i034_050.reset();
    i034_060.reset();
    i034_070.reset();
    i034_100.reset();
    i034_110.reset();
    i034_120.reset();
    i034_090.reset();
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
