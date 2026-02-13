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
#include <ReactorAsterix/cat034/Asterix034DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix034Report
 * @brief Container for decoded Category 034 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix034Report final : public AsterixReport {
    public:
        Asterix034Report() = default;
        ~Asterix034Report() override = default;

        /**
         * Decodes Cat 034 fields using the recursive template schema.
         */
        bool process_all_octets(
            std::string_view fspec,
            std::string_view& data,
            AsterixStatsData& stats,
            IAsterixCategoryHandler& parent) override;

        inline void reset() {
            // Reset common base fields (SourceIdentifier, TOD, etc.)
            AsterixReport::reset();

            i034_010.reset();
            i034_000.reset();
            i034_030.reset();
            i034_020.reset();
            i034_041.reset();
            i034_050.reset();
            i034_060.reset();
            i034_070.reset();
            i034_100.reset();
            i034_110.reset();
            i034_120.reset();
            i034_090.reset();
        };

        // --- Data Items (Standard Cat 034 v1.27) ---

        // FSPEC Octet 1
        I034_010_Handler i034_010; // Data Source Identifier
        I034_000_Handler i034_000; // Message Type
        I034_030_Handler i034_030; // Time of Message
        I034_020_Handler i034_020; // Sector Number
        I034_041_Handler i034_041; // Antenna Rotation Period
        I034_050_Handler i034_050; // System Configuration/Status
        I034_060_Handler i034_060; // System Processing Parameters

        // FSPEC Octet 2
        I034_070_Handler i034_070; // Plot Count Values
        I034_100_Handler i034_100; // Generic Polar Window
        I034_110_Handler i034_110; // Data Filter
        I034_120_Handler i034_120; // 3D Radar Position
        I034_090_Handler i034_090; // Collimation Error

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
