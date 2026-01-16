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
#include <cstdint>
#include <cstring>

namespace ReactorAsterix {

class FastBitReader {
    public:
        // Ptr is assumed valid because AsterixCategoryHandler checked size already
        explicit FastBitReader(const uint8_t* ptr) : data_(ptr) {}

        // Force the compiler to unroll loops by using templates for bit count
        // Usage: uint8_t val = reader.readBits<3>(bitOffset);
        template <int N>
            [[nodiscard]] inline uint8_t readBits(int& bitOffset) const {
                static_assert(N <= 8, "Use readByte for >8 bits");

                // This math is calculated at COMPILE TIME, not runtime
                constexpr uint8_t mask = (1 << N) - 1;

                // Logic: Shift down to align LSB, then mask
                // Correspond exactly to: (byte >> shift) & mask
                uint8_t value = (*data_ >> (bitOffset - N + 1)) & mask;

                // Update offset
                bitOffset -= N;
                if (bitOffset < 0) {
                    data_++;
                    bitOffset = 7;
                }
                return value;
            }

        // Optimized single bit read (Flags, SPI, etc)
        [[nodiscard]] inline bool readBit(int& bitOffset) const {
            bool val = (*data_ >> bitOffset) & 1;

            bitOffset--;
            if (bitOffset < 0) {
                data_++;       // Move to next byte
                bitOffset = 7; // Reset to MSB
            }
            return val;
        }

    private:
        mutable const uint8_t* data_;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
