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
#include <string_view>

// Library headers
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/cat001/Asterix001DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix001Report
 * @brief Container for decoded Category 001 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix001Report final : public AsterixMessage {
    public:
        Asterix001Report() = default;
        ~Asterix001Report() override = default;

       bool process_all_octets(
               std::string_view fspec, std::string_view& data,
               AsterixStatsData& stats);

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
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
