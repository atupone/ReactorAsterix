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
#include <ReactorAsterix/core/AsterixDataItemHandlerSP.h>
#include <ReactorAsterix/core/AsterixDecoderUtils.h>
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/cat048/Asterix048DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix048Report
 * @brief Container for decoded Category 048 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix048Report final : public AsterixReport {
    public:
        Asterix048Report() = default;
        ~Asterix048Report() override = default;

        /**
         * Decodes Cat 001 fields using the recursive template schema.
         */
        bool process_all_octets(
            std::string_view fspec,
            std::string_view& data,
            AsterixStatsData& stats,
            IAsterixCategoryHandler& parent) override;

        inline void reset() {
            // Reset common base fields (SourceIdentifier, TOD, etc.)
            AsterixReport::reset();

            i048_010.reset();
            i048_140.reset();
            i048_020.reset();
            i048_040.reset();
            i048_070.reset();
            i048_090.reset();
            i048_130.reset();

            i048_220.reset();
            i048_240.reset();
            i048_250.reset();
            i048_161.reset();
            i048_042.reset();
            i048_200.reset();
            i048_170.reset();

            i048_210.reset();
            i048_030.reset();
            i048_080.reset();
            i048_100.reset();
            i048_110.reset();
            i048_120.reset();
            i048_230.reset();

            i048_260.reset();
            i048_055.reset();
            i048_050.reset();
            i048_065.reset();
            i048_060.reset();
        };

        // --- Data Items (Standard Cat 048) ---

        // --- FSPEC Octet 1 ---
        I048_010_Handler i048_010;
        I048_140_Handler i048_140;
        I048_020_Handler i048_020;
        I048_040_Handler i048_040;
        I048_070_Handler i048_070;
        I048_090_Handler i048_090;
        I048_130_Handler i048_130;

        // --- FSPEC Octet 2 ---
        I048_220_Handler i048_220;
        I048_240_Handler i048_240;
        I048_250_Handler i048_250;
        I048_161_Handler i048_161;
        I048_042_Handler i048_042;
        I048_200_Handler i048_200;
        I048_170_Handler i048_170;

        // --- FSPEC Octet 3 ---
        I048_210_Handler i048_210;
        I048_030_Handler i048_030;
        I048_080_Handler i048_080;
        I048_100_Handler i048_100;
        I048_110_Handler i048_110;
        I048_120_Handler i048_120;
        I048_230_Handler i048_230;

        // --- FSPEC Octet 4 ---
        I048_260_Handler i048_260;
        I048_055_Handler i048_055;
        I048_050_Handler i048_050;
        I048_065_Handler i048_065;
        I048_060_Handler i048_060;

    private:
        inline bool decode_fspec(
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
