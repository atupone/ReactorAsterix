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
#include <cmath>

namespace ReactorAsterix {

/**
 * @class Asterix034Report
 * @brief Container for decoded Category 034 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix034Report : public AsterixMessage {
    public:
        Asterix034Report() : antennaSpeed(0.0f), sectorNumber(0), height(0) {};
        ~Asterix034Report() override = default;

        // Setters for the handlers to use
        void setMessageType(uint8_t type) { messageType = static_cast<MessageType>(type); }
        void setAntennaSpeed(float speed) { antennaSpeed = speed; };

        enum class MessageType : uint8_t {
            NORTH_MARKER = 1,
            SECTOR_CROSSING = 2,
            GEOGRAPHICAL_FILTER = 3,
            JAMMING_STROBE = 4,
            SOLAR_STORM = 5,
            SSR_JAMMING_STROBE = 6,
            MODE_S_JAMMING_STROBE = 7
        };

        double latitude{0.0};  // WGS-84 Decimal Degrees
        double longitude{0.0}; // WGS-84 Decimal Degrees

        float antennaSpeed{0.0f};

        uint8_t sectorNumber;
        uint16_t height;       // LSB = 1 m
        MessageType messageType{MessageType::NORTH_MARKER};
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
