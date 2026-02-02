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
#include <ReactorAsterix/core/AsterixDataItemHandlerFixedLength.h>

namespace ReactorAsterix {

// The context object
class Asterix034Report;

/**
 * @file Asterix034DataItemCollection.h
 * @brief Declares the specific handler classes for **ASTERIX Category 034** data items.
 *
 * Each class publicly inherits from an appropriate base class (`AsterixDataItemHandlerFixedLength`
 * or `AsterixDataItemHandlerExtendedLength`) and is responsible for decoding a single
 * data item into the `Asterix034Report` context object.
 */

// ----------------------------------------------------------------------------------
// ASTERIX CAT 034 DATA ITEM HANDLERS
// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I034/010, Data Source Identifier.
 *
 * This mandatory, 2-byte item provides the System Area Code (SAC) and System
 * Identification Code (SIC) to uniquely identify the data source.
 */
class I034_010_Handler final : public AsterixDataItemHandlerFixedLength<Asterix034Report> {
    public:
        static constexpr uint8_t FRN = 1;
        static constexpr bool mandatory = true;
        I034_010_Handler() : AsterixDataItemHandlerFixedLength<Asterix034Report>(2) {
            name = "I034/010 Data Source Identifier";
        }

        /**
         * @brief Decodes the 2-byte Data Source Identifier.
         * @param context The target `Asterix034Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix034Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for ASTERIX Data Item I034/000, Message Type.
 *
 * This mandatory, 1-byte item identifies the type of message being transmitted.
 */
class I034_000_Handler final: public AsterixDataItemHandlerFixedLength<Asterix034Report> {
    public:
        static constexpr uint8_t FRN = 2;
        static constexpr bool mandatory = true;
        I034_000_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I034/000, Message Type";
        }

        void decode(Asterix034Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for ASTERIX Data Item I034/030, Time of Day.
 *
 * This mandatory, 3-byte item represents the time of day, typically as the
 * number of seconds since midnight, in 1/128 second increments.
 */
class I034_030_Handler : public AsterixDataItemHandlerFixedLength<Asterix034Report> {
    public:
        static constexpr uint8_t FRN = 3;
        static constexpr bool mandatory = false;
        I034_030_Handler(): AsterixDataItemHandlerFixedLength(3) {
            name = "I034/030, Time of Day";
        }
        void decode(Asterix034Report& context,  std::string_view data) const override;
};

/**
 * @brief Handler for ASTERIX Data Item I034/020, Sector Number.
 *
 * This optional, 1-byte item specifies the sector number from which the data
 * originated, typically used in multi-sector radar systems.
 */
class I034_020_Handler final : public AsterixDataItemHandlerFixedLength<Asterix034Report> {
    public:
        static constexpr uint8_t FRN = 4;
        I034_020_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I034/020, Sector Number";
        }
        void decode(Asterix034Report& context,  std::string_view data) const override;
};

class I034_041_Handler final : public AsterixDataItemHandlerFixedLength<Asterix034Report> {
    public:
        static constexpr uint8_t FRN = 5; // Standard FRN for Rotation Period
        I034_041_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I034/041, Antenna Rotation Period";
        }
        void decode(Asterix034Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for ASTERIX Data Item I034/120, 3D-Position of Data Source
 *
 * This optional, 8-byte item specifies the location of the Radar
 */
class I034_120_Handler final : public AsterixDataItemHandlerFixedLength<Asterix034Report> {
    public:
        static constexpr uint8_t FRN = 11;
        I034_120_Handler() : AsterixDataItemHandlerFixedLength(8) {
            name = "I034/120, 3D-Position of Data Source";
        }
        void decode(Asterix034Report& context,  std::string_view data) const override;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
