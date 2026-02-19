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
class Asterix001Report;

/**
 * @file Asterix1DataItemCollection.h
 * @brief Declares the specific handler classes for **ASTERIX Category 1** data items.
 *
 * Each class publicly inherits from an appropriate base class (`AsterixDataItemHandlerFixedLength`
 * or `AsterixDataItemHandlerExtendedLength`) and is responsible for decoding a single
 * data item into the `Asterix001Report` context object.
 */

// ----------------------------------------------------------------------------------
// ASTERIX CAT 001 DATA ITEM HANDLERS
// ----------------------------------------------------------------------------------

/**
 * @brief Handler for I001/010, Data Source Identifier.
 * A mandatory, fixed-length (2-byte) item providing the SAC and SIC.
 */
class I001_010_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 1;
        I001_010_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I001/010 Data Source Identifier";
            mandatory = true;
        }

        /**
         * @brief Decodes the 2-byte Data Source Identifier.
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t decode(std::string_view data) override;

        uint8_t sac;
        uint8_t sic;
};

/**
 * @brief Handler for I001/020, Target Report Descriptor.
 * A mandatory, extended-length item describing the nature and status of the report.
 */
class I001_020_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 2;
        I001_020_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I001/020 Target Report Descriptor";
            mandatory = true;
        }

        /**
         * @brief Decodes the Target Report Descriptor (TRT, SPI, EMG, etc.).
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t  decode(std::string_view data) override;

        inline void reset() {
            AsterixDataItemHandlerExtendedLength::reset();
            typ = false;
            sim = false;
            ssrpsr = SSRPSR_T::NO_DETECTION;
            ant = false;
            spi = false;
            rab = false;
            tst = false;
            ds1ds2 = DS1DS2_T::DEFAULT;
            me = false;
            mi = false;
            extra = false;
        };

        // Enumeration for SSR/PSR
        enum class SSRPSR_T : uint8_t {
            NO_DETECTION = 0,
            SOLE_PRIMARY_DETECTION = 1,
            SOLE_SECONDARY_DETECTION = 2,
            COMBINED_PRIMARY_AND_SECONDARY_DETECTION = 3
        };

        enum class DS1DS2_T : uint8_t {
            DEFAULT = 0,
            UNLAWFUL_INTERFERENCE = 1,
            RADIO_COMMUNICATION_FAILURE = 2,
            EMERGENCY = 3
        };

        bool typ{false};
        bool sim{false};
        SSRPSR_T ssrpsr{SSRPSR_T::NO_DETECTION};
        bool ant{false};
        bool spi{false};
        bool rab{false};

        bool tst{false};
        DS1DS2_T ds1ds2{DS1DS2_T::DEFAULT};
        bool me{false};
        bool mi{false};

        bool extra{false};
};

/**
 * @brief Handler for I001/040, Measured Position in Polar Coordinates.
 * An optional, fixed-length (4-byte) item for Range and Azimuth.
 */
class I001_040_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 3;
        I001_040_Handler() : AsterixDataItemHandlerFixedLength(4) {
            name = "I001/040 Measured Position (Polar)";
        };

        /**
         * @brief Decodes the 4-byte measured range and azimuth.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t  decode(std::string_view data) override;

        uint16_t range;
        uint16_t azimuth;
};

/**
 * @brief Handler for I001/070, Mode-3/A Code in Octal Representation.
 * An optional, fixed-length (2-byte) item for the aircraft transponder code.
 */
class I001_070_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 4;
        I001_070_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I001/070 Mode-3/A Code";
        }

        /**
         * @brief Decodes the 2-byte Mode-3/A code.
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t  decode(std::string_view data) override;

        uint16_t code;
        bool validated;
        bool garbled;
        bool local;
};

/**
 * @brief Handler for I001/090, Mode-C Code (Flight Level) in Binary Representation.
 * An optional, fixed-length (2-byte) item for the target's pressure altitude.
 */
class I001_090_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 5;
        I001_090_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I001/090 Mode-C Code (Flight Level)";
        }

        /**
         * @brief Decodes the 2-byte Mode-C code and converts it to a height in meters.
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t  decode(std::string_view data) override;

        int16_t height;
        bool validated;
        bool garbled;
};

/**
 * @brief Handler for I001/130, Radar Plot Characteristics.
 * An optional, extended-length item for supplementary plot characteristics.
 */
class I001_130_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 6;
        I001_130_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I001/130 Radar Plot Characteristics";
        }
};

/**
 * @brief Handler for I001/141, Truncated Time of Day.
 * An optional, fixed-length (2-byte) item for the time of detection.
 */
class I001_141_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 7;
        I001_141_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I001/141 Truncated Time of Day";
        }
        /**
         * @brief Decodes the 2-byte Truncated Time of Day (TOD).
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t  decode(std::string_view data) override;

        uint16_t todLSP;
};

/**
 * @brief Handler for I001/050, Mode-2 Code in Octal Representation.
 * An optional, fixed-length (2-byte) item (usually for military identification).
 */
class I001_050_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 8;
        I001_050_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I001/050 Mode-2 Code";
        }
};

class I001_120_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 9;
        I001_120_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I001/120 Measured Radial Doppler Speed";
        }
};

/**
 * @brief Handler for I001/131, Received Power.
 * An optional, fixed-length (1-byte) item for the received signal strength.
 */
class I001_131_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 10;
        I001_131_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I001/131 Received Power";
        }
};

class I001_080_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 11;
        I001_080_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I001/080 Mode-3/A Code in Confidence Indicator";
        }
};

class I001_100_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 12;
        I001_100_Handler() : AsterixDataItemHandlerFixedLength(4) {
            name = "I001/100 Mode-C Code in Confidence Indicator"; 
        }
};

class I001_060_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 13;
        I001_060_Handler() : AsterixDataItemHandlerFixedLength(2) { 
            name = "I001/060 Mode-2 Code in Confidence Indicator"; 
        }
};

/**
 * @brief I001/030 Warning/Error Conditions
 * Corrected: Inherits from ExtendedLength.
 */
class I001_030_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 14;
        I001_030_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) { 
            name = "I001/030 Warning/Error Conditions"; 
        }
};

/**
 * @brief Handler for I001/150, Presence of X-Puls.
 * An optional, fixed-length (1-byte) item indicating the presence of an X-Pulse.
 */
class I001_150_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 15;
        I001_150_Handler() : AsterixDataItemHandlerFixedLength(1) {
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
