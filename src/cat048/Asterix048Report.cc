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
#include <ReactorAsterix/core/AsterixDataItemHandlerSP.h>
#include <ReactorAsterix/core/SourceStateManager.h>
#include <ReactorAsterix/cat048/Asterix048Handler.h>

namespace ReactorAsterix {

bool Asterix048Report::decode_fspec(
        std::string_view fspec,
        std::string_view& data,
        AsterixStatsData& stats)
{
    AsterixDataItemHandlerSP sp;
    AsterixDataItemHandlerSP re;

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(fspec.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    // --- Octet 1 ---
    auto octet1 = std::tie(i048_010, i048_140, i048_020, i048_040, i048_070, i048_090, i048_130);
    if (!decode_octet_inline(reader, bit, octet1, data, stats)) return false;
    if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

    // --- Octet 2 ---
    auto octet2 = std::tie(i048_220, i048_240, i048_250, i048_161, i048_042, i048_200, i048_170);
    if (!decode_octet_inline(reader, bit, octet2, data, stats)) return false;
    if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

    // --- Octet 3 ---
    auto octet3 = std::tie(i048_210, i048_030, i048_080, i048_100, i048_110, i048_120, i048_230);
    if (!decode_octet_inline(reader, bit, octet3, data, stats)) return false;
    if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

    // --- Octet 4 ---
    auto octet4 = std::tie(i048_260, i048_055, i048_050, i048_065, i048_060, sp,       re);
    if (!decode_octet_inline(reader, bit, octet4, data, stats)) return false;
    if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

    stats.uninterpretedItems++;
    return true;
}

bool Asterix048Report::process_all_octets(
        std::string_view fspec,
        std::string_view& data,
        AsterixStatsData& stats,
        IAsterixCategoryHandler& parent)
{
    if (!decode_fspec(fspec, data, stats)) {
        return false;
    }

    // 010 - Source ID side effect
    if (i048_010.presence) [[likely]] {
        uint8_t sac = i048_010.sac;
        uint8_t sic = i048_010.sic;

        auto& concreteParent = static_cast<Asterix048Handler&>(parent);

        const SourceRecord* record = concreteParent.getSourceRecordCached(sac, sic);
        setSourceIdentifier(sac, sic, record);
    }

    // 020 - Abnormal checks
    if (i048_020.presence && (i048_020.sim || i048_020.rab || i048_020.tst)) {
        stats.uninterpretedItems++;
        return false;
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
