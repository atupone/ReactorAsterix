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
template <typename T>
class AsterixDataItemHandlerExtendedLength : public AsterixDataItemHandlerBase<T> {
    public:
        /**
         * @brief Constructor
         * @param k The initial length of the item (usually 1).
         * @param i The increment size if the FX bit is set (usually 1).
         */
        explicit AsterixDataItemHandlerExtendedLength(uint8_t _k, uint8_t _i)
            : k(_k), i(_i) {}
        ~AsterixDataItemHandlerExtendedLength() override = default;

        /**
         * @brief Calculates the total size by scanning for the FX bit (LSB).
         * Matches the signature in IAsterixDataItemHandler.h.
         */
        size_t getSize(std::string_view data) const final;

    protected:
        uint8_t k, i;
};

template <typename T>
size_t AsterixDataItemHandlerExtendedLength<T>::getSize(std::string_view data) const {
    // k is the initial length (e.g., 1 byte)
    size_t currentPos = k - 1;

    // The item has variable length. The loop continues as long as the FX bit (LSB) is set.
    while (currentPos < data.size()) {
        const uint8_t byte = static_cast<uint8_t>(data[currentPos]);
        if (!(byte & 0x01)) break; // FX bit not set, we are at the end
        currentPos += i;
    }

    size_t totalSize = currentPos + 1;

    // Safety check: if the loop finished because we ran out of data
    // rather than finding a 0 FX bit, the packet is malformed.
    return (totalSize <= data.size()) ? totalSize : 0;
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
