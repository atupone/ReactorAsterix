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

#pragma once

// Inherits from
#include <ReactorAsterix/core/AsterixReport.h>

// System headers
#include <string_view>
#include <vector>

// Library headers
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/core/AsterixDecoderUtils.h>
#include <ReactorAsterix/cat002/Asterix002DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix002Report
 * @brief Container for decoded Category 002 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix002Report final : public AsterixReport {
    public:
        Asterix002Report() = default;
        ~Asterix002Report() override = default;

        /**
         * Decodes Cat 002 fields using the recursive template schema.
         */
        bool process_all_octets(
            std::string_view fspec,
            std::string_view& data,
            AsterixStatsData& stats,
            IAsterixCategoryHandler& parent) override;

        inline void reset() {
            // Reset common base fields (SourceIdentifier, TOD, etc.)
            AsterixReport::reset();

            i002_010.reset();
            i002_000.reset();
            i002_020.reset();
            i002_030.reset();
            i002_041.reset();
            i002_050.reset();
            i002_060.reset();
            i002_070.reset();
            i002_100.reset();
            i002_090.reset();
            i002_080.reset();
        };

        // --- Data Items (Standard Cat 002) ---

        // --- FSPEC Octet 1 ---
        I002_010_Handler i002_010;
        I002_000_Handler i002_000;
        I002_020_Handler i002_020;
        I002_030_Handler i002_030;
        I002_041_Handler i002_041;
        I002_050_Handler i002_050;
        I002_060_Handler i002_060;

        // --- FSPEC Octet 2 ---
        I002_070_Handler i002_070;
        I002_100_Handler i002_100;
        I002_090_Handler i002_090;
        I002_080_Handler i002_080;

    private:
        inline bool decode_fspec(
                std::string_view fspec,
                std::string_view& data,
                AsterixStatsData& stats)
        {
            const uint8_t* raw = reinterpret_cast<const uint8_t*>(fspec.data());
            FastBitReader reader(raw);
            int bit = 7; // Start at MSB

            // --- Octet 1 ---
            auto octet1 = std::tie(i002_010, i002_000, i002_020, i002_030, i002_041, i002_050, i002_060);
            if (!decode_octet_inline(reader, bit, octet1, data, stats)) return false;
            if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

            // --- Octet 2 ---
            auto octet2 = std::tie(i002_070, i002_100, i002_090, i002_080, dummy,    dummy,    dummy);
            if (!decode_octet_inline(reader, bit, octet2, data, stats)) return false;
            if (!reader.readBit(bit)) return true; // FX bit: if 0, we are done

            stats.uninterpretedItems++;
            return true;
        };
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
