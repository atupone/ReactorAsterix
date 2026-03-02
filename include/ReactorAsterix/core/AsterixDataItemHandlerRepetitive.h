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

// Inherits from the main interface
#include <ReactorAsterix/core/AsterixDataItemHandlerBase.h>

// System headers
#include <cstdint>

namespace ReactorAsterix {

/**
 * @class AsterixDataItemHandlerExtendedLength
 * @brief Helper template for ASTERIX data items that have a variable length
 * determined by an extension bit (FX) in the Least Significant Bit (LSB).
 */
class AsterixDataItemHandlerRepetitive : public AsterixDataItemHandlerBase {
    public:
        /**
         * @brief Constructor
         * @param k Size of the repetitive item.
         */
        explicit AsterixDataItemHandlerRepetitive(uint8_t _k)
            : k(_k) {}
        ~AsterixDataItemHandlerRepetitive() override = default;

        /**
         * @brief Calculates the total size by scanning for the FX bit (LSB).
         * Matches the signature in IAsterixDataItemHandler.h.
         */
        size_t decode(std::string_view data) {
            AsterixDataItemHandlerBase::decode(data);

            size_t totalSize = 1;
            if (data.size() < 1) {
                return 0;
            }

            const uint8_t rep = static_cast<uint8_t>(data[0]);
            if (rep == 0) {
                return 0;
            }

            totalSize += rep * k;

            // Safety check: if the loop finished because we ran out of data
            // rather than finding a 0 FX bit, the packet is malformed.
            return (totalSize <= data.size()) ? totalSize : 0;
        }

    protected:
        uint8_t k;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
