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

// Define the static member (required in C++)
std::vector<uint8_t> Asterix001Report::mandatory_mask;

auto Asterix001Report::get_schema() {
    return std::make_tuple(
        std::tie(i001_010, i001_020, i001_040, i001_070, i001_090, i001_130, i001_141),
        std::tie(i001_050, i001_120, i001_131, i001_080, i001_100, i001_060, i001_030),
        std::tie(i001_150)
    );
}

void Asterix001Report::init_mandatory_mask() {
    if (!mandatory_mask.empty()) return;

    auto schema = get_schema();

    fill_mandatory_mask(schema, mandatory_mask);
}

bool Asterix001Report::process_all_octets(
        std::string_view fspec, std::string_view& data,
        AsterixStatsData& stats)
{
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(fspec.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    // Declarative Schema: tuple to match ASTERIX FSPEC layout
    auto schema = get_schema();

    // Decode using schema
    if (!decode_fspec_recursive(reader, bit, data, schema)) {
        return false;
    }

    // Fast Mandatory Validation
    for (size_t i = 0; i < mandatory_mask.size(); ++i) {
        uint8_t required = mandatory_mask[i];
        if (required == 0) continue; // No mandatory items in this octet

        // If the FSPEC is shorter than our mandatory mask, check if the
        // missing octets actually required anything.
        if (i >= fspec.size()) {
            return false; // Expected mandatory fields in octet i, but FSPEC ended
        }

        uint8_t actual = static_cast<uint8_t>(fspec[i]);

        // Standard ASTERIX bitwise check: (Required bits) AND NOT (Received bits)
        // If the result is non-zero, it means a bit set in 'required' was 0 in 'actual'.
        if ((required & ~actual) != 0) {
            return false;
        }
    }

    // 010 - Source ID side effect
    if (i001_010.presence) {
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
