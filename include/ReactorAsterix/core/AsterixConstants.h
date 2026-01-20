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

namespace ReactorAsterix::Constants {
    /**
     * @brief ASTERIX Block Header consists of 1 byte for Category
     * and 2 bytes for Length Indicator.
     */
    inline constexpr size_t HEADER_SIZE = 3;

    /**
     * @brief The absolute minimum size of a valid ASTERIX data block.
     * CAT(1) + LEN(2) + FSPEC(1) + DATA(1) = 5 bytes.
     */
    inline constexpr size_t MIN_BLOCK_SIZE = 5;

    /**
     * @brief Maximum number of ASTERIX categories (0-255).
     */
    constexpr size_t MAX_CATEGORIES = 256;

    constexpr size_t MAX_FSPEC_SIZE = 10;
    constexpr uint8_t FX_BIT = 0x01;

    // ASTERIX Time of Day constants (Category 002, 048, 062, etc.)

    // ASTERIX Time of Day (TOD) is 1/128 of a second
    constexpr uint32_t AST_TOD_UNITS_PER_SEC = 128;

    // Time Conversions
    constexpr uint64_t MS_PER_SEC = 1000ULL;
    constexpr uint64_t US_PER_SEC = 1000000ULL;
    constexpr uint64_t NS_PER_SEC = 1000000000ULL;

    // The exact number of nanoseconds in one ASTERIX TOD unit
    // 1,000,000,000 / 128 = 7,812,500
    constexpr uint64_t NS_PER_AST_TOD_UNIT = NS_PER_SEC / AST_TOD_UNITS_PER_SEC;

    // Midnight Logic
    constexpr uint32_t SECONDS_PER_DAY = 86400;
    constexpr uint32_t AST_TOD_UNITS_PER_DAY = SECONDS_PER_DAY * AST_TOD_UNITS_PER_SEC; // 11,059,200
    constexpr uint32_t AST_TOD_HALFDAY_UNITS = AST_TOD_UNITS_PER_DAY / 2;             // 5,529,600

}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
