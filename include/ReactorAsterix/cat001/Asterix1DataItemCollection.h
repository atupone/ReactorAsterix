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

namespace ReactorAsterix {

// The context object
class Asterix1Report;

/**
 * @file Asterix1DataItemCollection.h
 * @brief Declares the specific handler classes for **ASTERIX Category 1** data items.
 *
 * Each class publicly inherits from an appropriate base class (`AsterixDataItemHandlerFixedLength`
 * or `AsterixDataItemHandlerExtendedLength`) and is responsible for decoding a single
 * data item into the `Asterix1Report` context object.
 */

// ----------------------------------------------------------------------------------
// ASTERIX CAT 001 DATA ITEM HANDLERS
// ----------------------------------------------------------------------------------

/**
 * @brief Handler for I001/010, Data Source Identifier.
 * A mandatory, fixed-length (2-byte) item providing the SAC and SIC.
 */
class I001_010_Handler final : public AsterixDataItemHandlerFixedLength<Asterix1Report> {
    public:
        I001_010_Handler() : AsterixDataItemHandlerFixedLength<Asterix1Report>(2) {
            name = "I001/010 Data Source Identifier";
            mandatory = true;
        }

        /**
         * @brief Decodes the 2-byte Data Source Identifier.
         * @param context The target `Asterix1Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix1Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I001/020, Target Report Descriptor.
 * A mandatory, extended-length item describing the nature and status of the report.
 */
class I001_020_Handler final : public AsterixDataItemHandlerExtendedLength<Asterix1Report> {
    public:
        I001_020_Handler() : AsterixDataItemHandlerExtendedLength<Asterix1Report>(1, 1) {
            name = "I001/020 Target Report Descriptor";
            mandatory = true;
        }

        /**
         * @brief Decodes the Target Report Descriptor (TRT, SPI, EMG, etc.).
         * @param context The target `Asterix1Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix1Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I001/040, Measured Position in Polar Coordinates.
 * An optional, fixed-length (4-byte) item for Range and Azimuth.
 */
class I001_040_Handler final : public AsterixDataItemHandlerFixedLength<Asterix1Report> {
    public:
        I001_040_Handler() : AsterixDataItemHandlerFixedLength<Asterix1Report>(4) {
            name = "I001/040 Measured Position (Polar)";
        };

        /**
         * @brief Decodes the 4-byte measured range and azimuth.
         * @param context The target `Asterix1Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix1Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I001/070, Mode-3/A Code in Octal Representation.
 * An optional, fixed-length (2-byte) item for the aircraft transponder code.
 */
class I001_070_Handler final : public AsterixDataItemHandlerFixedLength<Asterix1Report> {
    public:
        I001_070_Handler() : AsterixDataItemHandlerFixedLength<Asterix1Report>(2) {
            name = "I001/070 Mode-3/A Code";
        }

        /**
         * @brief Decodes the 2-byte Mode-3/A code.
         * @param context The target `Asterix1Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix1Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I001/090, Mode-C Code (Flight Level) in Binary Representation.
 * An optional, fixed-length (2-byte) item for the target's pressure altitude.
 */
class I001_090_Handler final : public AsterixDataItemHandlerFixedLength<Asterix1Report> {
    public:
        I001_090_Handler() : AsterixDataItemHandlerFixedLength<Asterix1Report>(2) {
            name = "I001/090 Mode-C Code (Flight Level)";
        }

        /**
         * @brief Decodes the 2-byte Mode-C code and converts it to a height in meters.
         * @param context The target `Asterix1Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix1Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I001/130, Radar Plot Characteristics.
 * An optional, extended-length item for supplementary plot characteristics.
 */
class I001_130_Handler final : public AsterixDataItemHandlerExtendedLength<Asterix1Report> {
    public:
        I001_130_Handler() : AsterixDataItemHandlerExtendedLength<Asterix1Report>(1, 1) {
            name = "I001/130 Radar Plot Characteristics";
        }

        // The decode function is not implemented using the
        // implementation on the parent class discards info.
};

/**
 * @brief Handler for I001/141, Truncated Time of Day.
 * An optional, fixed-length (2-byte) item for the time of detection.
 */
class I001_141_Handler final : public AsterixDataItemHandlerFixedLength<Asterix1Report> {
    public:
        I001_141_Handler() : AsterixDataItemHandlerFixedLength<Asterix1Report>(2) {
            name = "I001/141 Truncated Time of Day";
        }
        /**
         * @brief Decodes the 2-byte Truncated Time of Day (TOD).
         * @param context The target `Asterix1Report` object.
         * @param data The raw data buffer.
         */
        void decode(Asterix1Report& context, std::string_view data) const override;
};

/**
 * @brief Handler for I001/050, Mode-2 Code in Octal Representation.
 * An optional, fixed-length (2-byte) item (usually for military identification).
 */
class I001_050_Handler final : public AsterixDataItemHandlerFixedLength<Asterix1Report> {
    public:
        I001_050_Handler() : AsterixDataItemHandlerFixedLength<Asterix1Report>(2) {
            name = "I001/050 Mode-2 Code";
        }
};

/**
 * @brief Handler for I001/131, Received Power.
 * An optional, fixed-length (1-byte) item for the received signal strength.
 */
class I001_131_Handler final : public AsterixDataItemHandlerFixedLength<Asterix1Report> {
    public:
        I001_131_Handler() : AsterixDataItemHandlerFixedLength<Asterix1Report>(1) {
            name = "I001/131 Received Power";
        }
};

/**
 * @brief Handler for I001/150, Presence of X-Puls.
 * An optional, fixed-length (1-byte) item indicating the presence of an X-Pulse.
 */
class I001_150_Handler final : public AsterixDataItemHandlerFixedLength<Asterix1Report> {
    public:
        I001_150_Handler() : AsterixDataItemHandlerFixedLength<Asterix1Report>(1) {
            name = "I001/150 Presence of X-Pulse";
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
