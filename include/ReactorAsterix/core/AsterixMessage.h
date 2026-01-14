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

// System headers
#include <cstddef>
#include <cstdint>

// Library headers
#include <ReactorAsterix/core/SourceIdentifier.h>

namespace ReactorAsterix {

/**
 * @class AsterixMessage
 * @brief The base class for all decoded ASTERIX data.
 * focusing purely on shared metadata like Source ID and Reception Time.
 */
class AsterixMessage {
    public:
        AsterixMessage() = default;
        virtual ~AsterixMessage() = default;

        // Uniquely identifies the radar station
        SourceIdentifier sourceIdentifier;

        // The time the message was received
        uint32_t TOD;

        // Reusable setter used by Ixxx/010 Handlers across all categories
        void setSourceIdentifier(uint8_t sac, uint8_t sic) {
            sourceIdentifier = {sac, sic};
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
