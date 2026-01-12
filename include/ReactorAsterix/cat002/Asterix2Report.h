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
#include "ReactorAsterix/core/AsterixMessage.h"

// System headers
#include <cstdint>
#include <cmath>
#include <optional>

namespace ReactorAsterix {

/**
 * @class Asterix2Report
 * @brief Container for decoded Category 002 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix2Report : public AsterixMessage {
    public:
        Asterix2Report() = default;
        ~Asterix2Report() override = default;

      float antennaSpeed;

      void setAntennaSpeed(float speed) { antennaSpeed = speed; };
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
