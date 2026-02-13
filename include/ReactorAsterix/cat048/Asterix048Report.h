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

        void reset();

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
        bool decode_fspec(
                std::string_view fspec,
                std::string_view& data,
                AsterixStatsData& stats);
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
