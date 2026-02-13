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
#include <ReactorAsterix/core/AsterixDataItemHandlerSP.h>
#include <ReactorAsterix/core/SourceStateManager.h>
#include <ReactorAsterix/cat034/Asterix034Handler.h>

namespace ReactorAsterix {

bool Asterix034Report::decode_fspec(
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
    auto octet1 = std::tie(i034_010, i034_000, i034_030, i034_020, i034_041, i034_050, i034_060);
    if (!decode_octet_inline(reader, bit, octet1, data, stats)) return false;
    if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

    // --- Octet 2 ---
    auto octet2 = std::tie(i034_070, i034_100, i034_110, i034_120, i034_090, re,       sp);
    if (!decode_octet_inline(reader, bit, octet2, data, stats)) return false;
    if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

    stats.uninterpretedItems++;
    return true;
}

bool Asterix034Report::process_all_octets(
        std::string_view fspec,
        std::string_view& data,
        AsterixStatsData& stats,
        IAsterixCategoryHandler& parent)
{
    if (!decode_fspec(fspec, data, stats)) {
        return false;
    }

    // 010 - Source ID side effect
    if (i034_010.presence) [[likely]] {
        uint8_t sac = i034_010.sac;
        uint8_t sic = i034_010.sic;

        auto& concreteParent = static_cast<Asterix034Handler&>(parent);

        const SourceRecord* record = concreteParent.getSourceRecordCached(sac, sic);
        setSourceIdentifier(sac, sic, record);
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
