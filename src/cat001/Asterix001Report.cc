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

bool Asterix001Report::decode_fspec(
        std::string_view fspec,
        std::string_view& data,
        AsterixStatsData& stats)
{
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(fspec.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    // --- Octet 1 ---
    auto octet1 = std::tie(i001_010, i001_020, i001_040, i001_070, i001_090, i001_130, i001_141);
    if (!decode_octet_inline(reader, bit, octet1, data, stats)) return false;
    if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

    // --- Octet 2 ---
    auto octet2 = std::tie(i001_050, i001_120, i001_131, i001_080, i001_100, i001_060, i001_030);
    if (!decode_octet_inline(reader, bit, octet2, data, stats)) return false;
    if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

    // --- Octet 3 ---
    auto octet3 = std::tie(i001_150, dummy,    dummy,     dummy,   dummy,    dummy,    dummy);
    if (!decode_octet_inline(reader, bit, octet3, data, stats)) return false;
    if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

    stats.uninterpretedItems++;
    return true;
}

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
    if (i001_020.extra) {
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
