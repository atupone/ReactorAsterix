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

// Define the static member (required in C++)
std::vector<uint8_t> Asterix048Report::mandatory_mask;

auto Asterix048Report::get_schema() {
    return std::make_tuple(
        std::tie(i048_010, i048_140, i048_020, i048_040, i048_070, i048_090, i048_130),
        std::tie(i048_220, i048_240, i048_250, i048_161, i048_042, i048_200, i048_170),
        std::tie(i048_210, i048_030, i048_080, i048_100, i048_110, i048_120, i048_230),
        std::tie(i048_260, i048_055, i048_050, i048_065, i048_060)
    );
}

void Asterix048Report::init_mandatory_mask() {
    if (!mandatory_mask.empty()) return;

    auto schema = get_schema();

    fill_mandatory_mask(schema, mandatory_mask);
}

bool Asterix048Report::process_all_octets(
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
    if (i048_010.presence) {
        setSourceIdentifier(i048_010.sac, i048_010.sic);
    }

    // 020 - Abnormal checks
    if (i048_020.presence && (i048_020.sim || i048_020.rab || i048_020.tst)) {
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
