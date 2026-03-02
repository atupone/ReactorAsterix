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
#include <arpa/inet.h>
#include <cstring>

namespace ReactorAsterix {

template <typename T>
T readBigEndian(const void* src) {
    // Copy to a local variable of type T
    T val;
    std::memcpy(&val, src, sizeof(T));

    if constexpr (sizeof(T) == 2) {
        return static_cast<T>(ntohs(static_cast<uint16_t>(val)));
    }
    if constexpr (sizeof(T) == 4) {
        return static_cast<T>(ntohl(static_cast<uint32_t>(val)));
    }
    return val;
}

template<typename T>
inline T decodeBigEndian(std::string_view data) {
    if (data.size() < sizeof(T)) return 0;

    T val;
    std::memcpy(&val, data.data(), sizeof(T));

    if constexpr (sizeof(T) == 2) return ntohs(val);
    if constexpr (sizeof(T) == 4) return ntohl(val);
    if constexpr (sizeof(T) == 8) return be64toh(val);
    return val;
}

/**
 * @brief Decodes a 3-byte Big-Endian signed integer.
 * Hardcoded for 24-bit ASTERIX fields (Coordinates).
 */
inline int32_t decode24BitSigned(std::string_view data) {
    // data.substr(offset, 3) is assumed
    uint32_t val =
        (static_cast<uint8_t>(data[0]) << 16) |
        (static_cast<uint8_t>(data[1]) << 8)  |
        (static_cast<uint8_t>(data[2]));

    // If bit 23 is set, it's negative. Sign-extend to 32 bits.
    if (val & 0x800000) {
        val |= 0xFF000000;
    }
    return static_cast<int32_t>(val);
}

}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
