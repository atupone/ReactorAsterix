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
#include <ReactorAsterix/core/AsterixMessage.h>

// System headers
#include <cstdint>
#include <string_view>

// Library headers
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/core/FastBitReader.h>
#include <ReactorAsterix/cat048/Asterix048DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix048Report
 * @brief Container for decoded Category 048 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix048Report final : public AsterixMessage {
    public:
       Asterix048Report() = default;
       ~Asterix048Report() override = default;

       bool process_all_octets(
               std::string_view fspec, std::string_view& data,
               AsterixStatsData& stats);

       // --- FSPEC Octet 1 ---
       bool i048_010_presence{false};
       I048_010_Handler i048_010;

       bool i048_140_presence{false};
       I048_140_Handler i048_140;

       bool i048_020_presence{false};
       I048_020_Handler i048_020;

       bool i048_040_presence{false};
       I048_040_Handler i048_040;

       bool i048_070_presence{false};
       I048_070_Handler i048_070;

       bool i048_090_presence{false};
       I048_090_Handler i048_090;

       bool i048_130_presence{false};
       I048_130_Handler i048_130;

       // --- FSPEC Octet 2 ---
       bool i048_220_presence{false};
       I048_220_Handler i048_220;

       bool i048_240_presence{false};
       I048_240_Handler i048_240;

       bool i048_250_presence{false};
       I048_250_Handler i048_250;

       bool i048_161_presence{false};
       I048_161_Handler i048_161;

       bool i048_042_presence{false};
       I048_042_Handler i048_042;

       bool i048_200_presence{false};
       I048_200_Handler i048_200;

       bool i048_170_presence{false};
       I048_170_Handler i048_170;

       // --- FSPEC Octet 3 ---
       bool i048_210_presence{false};
       I048_210_Handler i048_210;

       bool i048_030_presence{false};
       I048_030_Handler i048_030;

       bool i048_080_presence{false};
       I048_080_Handler i048_080;

       bool i048_100_presence{false};
       I048_100_Handler i048_100;

       bool i048_110_presence{false};
       I048_110_Handler i048_110;

       bool i048_120_presence{false};
       I048_120_Handler i048_120;

       bool i048_230_presence{false};
       I048_230_Handler i048_230;

       // --- FSPEC Octet 4 ---
       bool i048_260_presence{false};
       I048_260_Handler i048_260;

       bool i048_055_presence{false};
       I048_055_Handler i048_055;

       bool i048_050_presence{false};
       I048_050_Handler i048_050;

       bool i048_065_presence{false};
       I048_065_Handler i048_065;

       bool i048_060_presence{false};
       I048_060_Handler i048_060;
    };

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
