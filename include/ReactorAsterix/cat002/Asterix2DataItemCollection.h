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

// Library headers
#include "ReactorAsterix/core/AsterixDataItemHandlerBase.h"
#include "ReactorAsterix/core/AsterixDataItemHandlerFixedLength.h"
#include "ReactorAsterix/core/AsterixDataItemHandlerExtendedLength.h"

namespace ReactorAsterix {

// The context object
class Asterix2Report;

/**
 * @file Asterix1DataItemCollection.h
 * @brief Declares the specific handler classes for **ASTERIX Category 2** data items.
 *
 * Each class publicly inherits from an appropriate base class (`AsterixDataItemHandlerFixedLength`
 * or `AsterixDataItemHandlerExtendedLength`) and is responsible for decoding a single
 * data item into the `Asterix1Report` context object.
 */

// ----------------------------------------------------------------------------------
// ASTERIX CAT 002 DATA ITEM HANDLERS
// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I002/010, Data Source Identifier.
 *
 * This mandatory, 2-byte item provides the System Area Code (SAC) and System
 * Identification Code (SIC) to uniquely identify the data source.
 */
class I002_010_Handler final : public AsterixDataItemHandlerFixedLength<Asterix2Report> {
    public:
        I002_010_Handler() : AsterixDataItemHandlerFixedLength<Asterix2Report>(2) {
            name = "I002/010 Data Source Identifier";
            mandatory = true;
        }

        /**
         * @brief Decodes the 2-byte Data Source Identifier.
         * @param context The target `Asterix1Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix2Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for ASTERIX Data Item I002/000, Message Type.
 *
 * This mandatory, 1-byte item identifies the type of message being transmitted.
 */
class I002_000_Handler final: public AsterixDataItemHandlerFixedLength<Asterix2Report> {
    public:
        I002_000_Handler() : AsterixDataItemHandlerFixedLength(1) {
            mandatory = true;
            name      = "I002/000, Message Type";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I002/020, Sector Number.
 *
 * This optional, 1-byte item specifies the sector number from which the data
 * originated, typically used in multi-sector radar systems.
 */
class I002_020_Handler final : public AsterixDataItemHandlerFixedLength<Asterix2Report> {
    public:
        I002_020_Handler() : AsterixDataItemHandlerFixedLength(1) {
            mandatory = false;
            name      = "I002/020, Sector Number";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I002/030, Time of Day.
 *
 * This mandatory, 3-byte item represents the time of day, typically as the
 * number of seconds since midnight, in 1/128 second increments.
 */
class I002_030_Handler : public AsterixDataItemHandlerFixedLength<Asterix2Report> {
    public:
        I002_030_Handler(): AsterixDataItemHandlerFixedLength(3) {
            mandatory = true;
            name      = "I002/030, Time of Day";
        }
        void decode(Asterix2Report& context,  std::string_view data) const override;
};

/**
 * @brief Handler for ASTERIX Data Item I002/041, Antenna Rotation Speed.
 *
 * This optional, 2-byte item specifies the rotation speed of the antenna
 * in revolutions per minute (RPM). The unit is 1/128 RPM.
 */
class I002_041_Handler final : public AsterixDataItemHandlerFixedLength<Asterix2Report> {
    public:
        I002_041_Handler() : AsterixDataItemHandlerFixedLength(2) {
            mandatory = false;
            name      = "I002/041, Antenna Rotation Speed";
        }
        void decode(Asterix2Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for ASTERIX Data Item I002/050, Station Configuration Status.
 *
 * This optional, extended-length item provides detailed status information
 * about the radar station configuration.
 */
class I002_050_Handler final : public AsterixDataItemHandlerExtendedLength<Asterix2Report> {
    public:
        I002_050_Handler(): AsterixDataItemHandlerExtendedLength(1, 1) {
            mandatory = false;
            name      = "I002/050, Station Configuration Status";
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
