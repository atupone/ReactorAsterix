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

namespace ReactorAsterix {

// The context object
class Asterix048Report;

/**
 * @file Asterix048DataItemCollection.h
 * @brief Declares the specific handler classes for **ASTERIX Category 048** data items.
 *
 * Each class publicly inherits from an appropriate base class (`AsterixDataItemHandlerFixedLength`
 * or `AsterixDataItemHandlerExtendedLength`) and is responsible for decoding a single
 * data item into the `Asterix1Report` context object.
 */

// ----------------------------------------------------------------------------------
// ASTERIX CAT 048 DATA ITEM HANDLERS
// ----------------------------------------------------------------------------------

/**
 * @brief Handler for I048/010, Data Source Identifier.
 * A mandatory, fixed-length (2-byte) item providing the SAC and SIC.
 */
class I048_010_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 1;
        static constexpr bool mandatory = true;
        I048_010_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(2) {
            name = "I048/010 Data Source Identifier";
        }

        /**
         * @brief Decodes the 2-byte Data Source Identifier.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix048Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for ASTERIX Data Item I048/140, Time of Day.
 *
 * This mandatory, 3-byte item represents the time of day, typically as the
 * number of seconds since midnight, in 1/128 second increments.
 */
class I048_140_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 2;
        static constexpr bool mandatory = false;
        I048_140_Handler(): AsterixDataItemHandlerFixedLength(3) {
            name = "I048/140, Time of Day";
        }
        void decode(Asterix048Report& context,  std::string_view data) const override;
};

/**
 * @brief Handler for I048/020, Target Report Descriptor.
 * A mandatory, extended-length item describing the nature and status of the report.
 */
class I048_020_Handler final : public AsterixDataItemHandlerExtendedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 3;
        static constexpr bool mandatory = true;
        I048_020_Handler() : AsterixDataItemHandlerExtendedLength<Asterix048Report>(1, 1) {
            name = "I048/020 Target Report Descriptor";
        }

        /**
         * @brief Decodes the Target Report Descriptor (TRT, SPI, EMG, etc.).
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix048Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I048/040, Measured Position in Polar Coordinates.
 * An optional, fixed-length (4-byte) item for Range and Azimuth.
 */
class I048_040_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 4;
        static constexpr bool mandatory = false;
        I048_040_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(4) {
            name = "I048/040 Measured Position in Polar Coordinates";
        };

        /**
         * @brief Decodes the 4-byte measured range and azimuth.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix048Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I048/070, Mode-3/A Code in Octal Representation.
 * An optional, fixed-length (2-byte) item for the aircraft transponder code.
 */
class I048_070_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 5;
        static constexpr bool mandatory = false;
        I048_070_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(2) {
            name = "I048/070 Mode-3/A Code";
        }

        /**
         * @brief Decodes the 2-byte Mode-3/A code.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix048Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I048/090, Flight Level in Binary Representation.
 * An optional, fixed-length (2-byte) item for the target's pressure altitude.
 */
class I048_090_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 6;
        static constexpr bool mandatory = false;
        I048_090_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(2) {
            name = "I048/090 Flight Level in Binary Representation";
        }

        /**
         * @brief Decodes the 2-byte Mode-C code and converts it to a height in meters.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix048Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I048/220, Aircraft Address.
 * An optional, fixed-length (3-byte) item for the Mode S Aircraft Address.
 */
class I048_220_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 8;
        static constexpr bool mandatory = false;
        I048_220_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(3) {
            name = "I048/220 Aircraft Address";
        }
};

/**
 * @brief Handler for I048/240, Aircraft Identification.
 * An optional, fixed-length (3-byte) item for the Mode S Aircraft Identification.
 */
class I048_240_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 9;
        static constexpr bool mandatory = false;
        I048_240_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(6) {
            name = "I048/240 Aircraft Identification";
        }
};

/**
 * @brief Handler for I048/250, Mode S MB Data.
 * A repetitive, (8-byte) item for the Mode S MB Data
 */
class I048_250_Handler final : public AsterixDataItemHandlerRepetitive<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 10;
        static constexpr bool mandatory = false;
        I048_250_Handler() : AsterixDataItemHandlerRepetitive<Asterix048Report>(8) {
            name = "I048/250 Mode S MB Data";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I048/161, Track Number
 */
class I048_161_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 11;
        static constexpr bool mandatory = false;
        I048_161_Handler(): AsterixDataItemHandlerFixedLength(2) {
            name = "I048/161, Track Number";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I048/042, Calculated Position in Cartesian Co-ordinates.
 */
class I048_042_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 12;
        static constexpr bool mandatory = false;
        I048_042_Handler(): AsterixDataItemHandlerFixedLength(4) {
            name = "I048/042, Calculated Position in Cartesian Co-ordinates";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I048/200, Calculated Track Velocity in Polar Co-ordinates.
 */
class I048_200_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 13;
        static constexpr bool mandatory = false;
        I048_200_Handler(): AsterixDataItemHandlerFixedLength(4) {
            name = "I048/200, Calculated Track Velocity in Polar Co-ordinates";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I048/170, Track Status
 */
class I048_170_Handler final : public AsterixDataItemHandlerExtendedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 14;
        static constexpr bool mandatory = false;
        I048_170_Handler() : AsterixDataItemHandlerExtendedLength<Asterix048Report>(1, 1) {
            name = "I048/170, Track Status";
        }
};

/**
 * @brief Handler for I048/100, Mode C Code and Confidence Indicator
 * An optional, fixed-length (4-byte) item.
 */
class I048_100_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 18;
        static constexpr bool mandatory = false;
        I048_100_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(4) {
            name = "I048/100 Mode-C Code and Confidence Indicator";
        }
};

/**
 * @brief Handler for I048/110, Height measured by a 3D Radar.
 * An optional, fixed-length (2-byte) item.
 */
class I048_110_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 19;
        static constexpr bool mandatory = false;
        I048_110_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(2) {
            name = "I048/110 Height measured by a 3D Radar";
        }

        /**
         * @brief Decodes the 2-byte Height from 3D Radar and converts it to a height in meters.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix048Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I048/230, Communication/ACAS Capability and Flight Status
 * An optional, fixed-length (2-byte) item.
 */
class I048_230_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 21;
        static constexpr bool mandatory = false;
        I048_230_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(2) {
            name = "I048/230 Communication/ACAS Capability and Flight Status";
        }
};

/**
 * @brief Handler for I048/055, Mode-1 Code in Octal Representation.
 * An optional, fixed-length (1-byte) item (usually for military identification).
 */
class I048_055_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 23;
        I048_055_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(1) {
            name = "I048/055 Mode-1 Code in Octal Representation";
        }
};

/**
 * @brief Handler for I048/050, Mode-2 Code in Octal Representation.
 * An optional, fixed-length (2-byte) item (usually for military identification).
 */
class I048_050_Handler final : public AsterixDataItemHandlerFixedLength<Asterix048Report> {
    public:
        static constexpr uint8_t FRN = 24;
        I048_050_Handler() : AsterixDataItemHandlerFixedLength<Asterix048Report>(2) {
            name = "I048/050 Mode-2 Code in Octal Representation";
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
