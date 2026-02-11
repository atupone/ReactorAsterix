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
#include <ReactorAsterix/core/AsterixDataItemHandlerExtendedLength.h>
#include <ReactorAsterix/core/AsterixDataItemHandlerRepetitive.h>

// System headers
#include <cstdlib>

namespace ReactorAsterix {

// The context object
class Asterix002Report;

/**
 * @file Asterix002DataItemCollection.h
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
class I002_010_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 1;
        static constexpr bool mandatory = true;
        I002_010_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I002/010 Data Source Identifier";
        }

        /**
         * @brief Decodes the 2-byte Data Source Identifier.
         * @param context The target `Asterix1Report` object.
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t decode(std::string_view data) override;

        uint8_t sac;
        uint8_t sic;
};

/**
 * @brief Handler for ASTERIX Data Item I002/000, Message Type.
 *
 * This mandatory, 1-byte item identifies the type of message being transmitted.
 */
class I002_000_Handler final: public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 2;
        static constexpr bool mandatory = true;
        I002_000_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name      = "I002/000, Message Type";
        }

        [[nodiscard]] size_t decode(std::string_view data) override;

        enum class MESSAGE_TYPE_T : uint8_t {
            NORTH_MARKER = 1,
            SECTOR_CROSSING = 2,
            SOUTH_MARKER = 3,
            ACTIVATION_OF_BLIND_ZONE_FILTERING = 8,
            STOP_OF_BLIND_ZONE_FILTERING = 9
        };

        MESSAGE_TYPE_T messageType{MESSAGE_TYPE_T::NORTH_MARKER};
};

/**
 * @brief Handler for ASTERIX Data Item I002/020, Sector Number.
 *
 * This optional, 1-byte item specifies the sector number from which the data
 * originated, typically used in multi-sector radar systems.
 */
class I002_020_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 3;
        I002_020_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name      = "I002/020, Sector Number";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I002/030, Time of Day.
 *
 * This mandatory, 3-byte item represents the time of day, typically as the
 * number of seconds since midnight, in 1/128 second increments.
 */
class I002_030_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 4;
        static constexpr bool mandatory = true;
        I002_030_Handler(): AsterixDataItemHandlerFixedLength(3) {
            name = "I002/030, Time of Day";
        }
        [[nodiscard]] size_t decode(std::string_view data) override;

        uint32_t TOD{0};
};

/**
 * @brief Handler for ASTERIX Data Item I002/041, Antenna Rotation Speed.
 *
 * This optional, 2-byte item specifies the rotation speed of the antenna
 * in revolutions per minute (RPM). The unit is 1/128 RPM.
 */
class I002_041_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 5;
        I002_041_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name      = "I002/041, Antenna Rotation Speed";
        }

        [[nodiscard]] size_t decode(std::string_view data) override;

        uint16_t speed{0};
};

/**
 * @brief Handler for ASTERIX Data Item I002/050, Station Configuration Status.
 *
 * This optional, extended-length item provides detailed status information
 * about the radar station configuration.
 */
class I002_050_Handler final : public AsterixDataItemHandlerExtendedLength{
    public:
        static constexpr uint8_t FRN = 6;
        I002_050_Handler(): AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I002/050, Station Configuration Status";
        }
};

/**
 * @brief I002/060 Station Processing Mode
 */
class I002_060_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 7;
        I002_060_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I002/060 Station Processing Mode";
        }
};

/**
 * @brief I002/070 Plot Count Values
 */
class I002_070_Handler final : public AsterixDataItemHandlerRepetitive {
    public:
        static constexpr uint8_t FRN = 8;
        I002_070_Handler() : AsterixDataItemHandlerRepetitive(2) {
            name = "I002/070 Plot Count Values";
        }
};

/**
 * @brief I002/100, Dynamic Window - Type 1
 */
class I002_100_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 9;
        I002_100_Handler() : AsterixDataItemHandlerFixedLength(8) {
            name = "I002/100, Dynamic Window - Type 1";
        }
};

/**
 * @brief I002/090, Collimation Error
 */
class I002_090_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 10;
        I002_090_Handler() : AsterixDataItemHandlerFixedLength(2) { 
            name = "I002/090, Collimation Error";
        }
};

/**
 * @brief I002/080 Warning/Error Conditions
 */
class I002_080_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 11;
        I002_080_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I002/080 Warning/Error Conditions";
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
