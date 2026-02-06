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
#include <ReactorAsterix/cat002/Asterix002DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix002Report
 * @brief Container for decoded Category 002 data.
 */
class Asterix002Report : public AsterixMessage {
    public:
        Asterix002Report() = default;
        ~Asterix002Report() override = default;

        /**
         * Decodes Cat 002 fields using the recursive template schema.
         */
        bool process_all_octets(
                std::string_view fspec,
                std::string_view& data,
                AsterixStats& stats);

        // --- Data Items (Standard Cat 002) ---

        // --- FSPEC Octet 1 ---
        bool i002_010_presence{false};
        I002_010_Handler i002_010;

        bool i002_000_presence{false};
        I002_000_Handler i002_000;

        bool i002_020_presence{false};
        I002_020_Handler i002_020;

        bool i002_030_presence{false};
        I002_030_Handler i002_030;

        bool i002_041_presence{false};
        I002_041_Handler i002_041;

        bool i002_050_presence{false};
        I002_050_Handler i002_050;

        bool i002_060_presence{false};
        I002_060_Handler i002_060;

        // --- FSPEC Octet 2 ---
        bool i002_070_presence{false};
        I002_070_Handler i002_070;

        bool i002_100_presence{false};
        I002_100_Handler i002_100;

        bool i002_090_presence{false};
        I002_090_Handler i002_090;

        bool i002_080_presence{false};
        I002_080_Handler i002_080;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
