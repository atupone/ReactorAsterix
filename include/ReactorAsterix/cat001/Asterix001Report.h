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
#include <ReactorAsterix/core/AsterixDecoderUtils.h>
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/cat001/Asterix001DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix001Report
 * @brief Container for decoded Category 001 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix001Report final : public AsterixReport {
    public:
        Asterix001Report() = default;
        ~Asterix001Report() override = default;

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

            i001_010.reset();
            i001_020.reset();
            i001_040.reset();
            i001_070.reset();
            i001_090.reset();
            i001_130.reset();
            i001_141.reset();
            i001_050.reset();
            i001_120.reset();
            i001_131.reset();
            i001_080.reset();
            i001_100.reset();
            i001_060.reset();
            i001_030.reset();
            i001_150.reset();
        };

        // --- Data Items (Standard Cat 001) ---

        // --- FSPEC Octet 1 ---
        I001_010_Handler i001_010;
        I001_020_Handler i001_020;
        I001_040_Handler i001_040;
        I001_070_Handler i001_070;
        I001_090_Handler i001_090;
        I001_130_Handler i001_130;
        I001_141_Handler i001_141;

        // --- FSPEC Octet 2 ---
        I001_050_Handler i001_050;
        I001_120_Handler i001_120;
        I001_131_Handler i001_131;
        I001_080_Handler i001_080;
        I001_100_Handler i001_100;
        I001_060_Handler i001_060;
        I001_030_Handler i001_030;

        // --- FSPEC Octet 3 ---
        I001_150_Handler i001_150;

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
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
