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
class AsterixDataItemHandlerExtendedLength : public AsterixDataItemHandlerBase {
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
         * @brief Hook for the mandatory first part of length k.
         * Default: does nothing, allowing for size-only decoding.
         */
        virtual void decodePrimary(std::string_view /*data*/) {}

        /**
         * @brief Hook for each subsequent extension of length i.
         * Default: does nothing.
         */
        virtual void decodeExtension(uint32_t /*index*/, std::string_view /*data*/) {}

        /**
         * @brief Calculates the total size by scanning for the FX bit (LSB).
         * Matches the signature in IAsterixDataItemHandler.h.
         */
        [[nodiscard]] size_t decode(std::string_view data) override final {
            AsterixDataItemHandlerBase::decode(data);

            // Boundary check for the Primary Part
            if (data.size() < k) return 0;

            // Call hook for Primary Part
            decodePrimary(data.substr(0, k));

            // k is the initial length (e.g., 1 byte)
            size_t currentPos = k - 1;
            uint32_t extCount = 0;

            // Iterate through extensions as long as FX bit is 1
            while (currentPos < data.size()) {
                const uint8_t lastByteOfBlock = static_cast<uint8_t>(data[currentPos]);

                if (!(lastByteOfBlock & 0x01)) break; // FX bit not set, we are at the end

                // Check if buffer has room for the next 'i' bytes
                if (data.size() < currentPos + 1 + i) return 0;

                extCount++;
                decodeExtension(extCount, data.substr(currentPos + 1, i));

                currentPos += i;
            }

            return currentPos + 1;
        }

    protected:
        uint8_t k, i;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
