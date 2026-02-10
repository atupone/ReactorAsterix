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
#include <ReactorAsterix/core/AsterixDataItemHandlerCompound.h>

// System headers
#include <cstdlib>

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
class I048_010_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 1;
        static constexpr bool mandatory = true;
        I048_010_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/010 Data Source Identifier";
        }

        /**
         * @brief Decodes the 2-byte Data Source Identifier.
         * @param data The raw data buffer.
         */
        void decode(std::string_view data) override;

        uint8_t sac{0};
        uint8_t sic{0};
};

/**
 * @brief Handler for ASTERIX Data Item I048/140, Time of Day.
 *
 * This mandatory, 3-byte item represents the time of day, typically as the
 * number of seconds since midnight, in 1/128 second increments.
 */
class I048_140_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 2;
        static constexpr bool mandatory = false;
        I048_140_Handler(): AsterixDataItemHandlerFixedLength(3) {
            name = "I048/140, Time of Day";
        }

        void decode(std::string_view data) override;

        uint32_t TOD;
};

/**
 * @brief Handler for I048/020, Target Report Descriptor.
 * A mandatory, extended-length item describing the nature and status of the report.
 */
class I048_020_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 3;
        static constexpr bool mandatory = true;
        I048_020_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I048/020 Target Report Descriptor";
        }
        /**
         * @brief Decodes the Target Report Descriptor (TRT, SPI, EMG, etc.).
         * @param data The raw data buffer.
         */
        void decode(std::string_view data) override;
        void reset();

        // Enumeration for TYP
        enum class TYP_T : uint8_t {
            NO_DETECTION = 0,
            SINGLE_PSR_DETECTION   = 1,
            SINGLE_SSR_DETECTION   = 2,
            SSR_PSR_DETECTION      = 3,
            SINGLE_MODES_ALL_CALL  = 4,
            SINGLE_MODES_ROLL_CALL = 5,
            MODES_ALL_CALL_PSR     = 6,
            MODES_ROLL_CALL_PSR    = 7
        };

        TYP_T typ{TYP_T::NO_DETECTION};
        bool  sim{false};
        bool  spi{false};
        bool  rab{false};
        bool  tst{false};
        bool  me{false};
};

/**
 * @brief Handler for I048/040, Measured Position in Polar Coordinates.
 * An optional, fixed-length (4-byte) item for Range and Azimuth.
 */
class I048_040_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 4;
        static constexpr bool mandatory = false;
        I048_040_Handler() : AsterixDataItemHandlerFixedLength(4) {
            name = "I048/040 Measured Position in Polar Coordinates";
        };

        /**
         * @brief Decodes the 4-byte measured range and azimuth.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        void decode(std::string_view data) override;

        uint16_t range;
        uint16_t azimuth;
};

/**
 * @brief Handler for I048/070, Mode-3/A Code in Octal Representation.
 * An optional, fixed-length (2-byte) item for the aircraft transponder code.
 */
class I048_070_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 5;
        I048_070_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/070 Mode-3/A Code";
        }

        /**
         * @brief Decodes the 2-byte Mode-3/A code.
         * @param data The raw data buffer.
         */
        void decode(std::string_view data) override;

        uint16_t code;
        bool validated;
        bool garbled;
        bool local;
};

/**
 * @brief Handler for I048/090, Flight Level in Binary Representation.
 * An optional, fixed-length (2-byte) item for the target's pressure altitude.
 */
class I048_090_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 6;
        I048_090_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/090 Flight Level in Binary Representation";
        }

        /**
         * @brief Decodes the 2-byte Mode-C code and converts it to a height in meters.
         * @param data The raw data buffer.
         */
        void decode(std::string_view data) override;

        int16_t height;
        bool validated;
        bool garbled;
};

/**
 * @brief Handler for I048/130, Radar Plot Characteristics.
 * An optional, compund item providing quality indicators of the plot.
 */
class I048_130_Handler final : public AsterixDataItemHandlerCompound {
    public:
        static constexpr uint8_t FRN = 7;

        class SRL final : public AsterixDataItemHandlerFixedLength {
            public:
                SRL() : AsterixDataItemHandlerFixedLength(1) {
                    name = "SSR Plot Runlength";
                }
        };

        class SRR final : public AsterixDataItemHandlerFixedLength {
            public:
                SRR() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Number of Received Replies for (M)SSR";
                }
        };

        class SAM final : public AsterixDataItemHandlerFixedLength {
            public:
                SAM() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Amplitude of (M)SSR reply";
                }
        };

        class PRL final : public AsterixDataItemHandlerFixedLength {
            public:
                PRL() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Primary Plot Runlength";
                }
        };

        class PAM final : public AsterixDataItemHandlerFixedLength {
            public:
                PAM() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Amplitude of Primary Plot";
                }
        };

        class RPD final : public AsterixDataItemHandlerFixedLength {
            public:
                RPD() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Difference in Range between PSR and SSR plot";
                }
        };

        class APD final : public AsterixDataItemHandlerFixedLength {
            public:
                APD() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Difference in Azimuth between PSR and SSR plot";
                }
        };

        SRL srl;
        SRR srr;
        SAM sam;
        PRL prl;
        PAM pam;
        RPD rpd;
        APD apd;

        I048_130_Handler()
            : AsterixDataItemHandlerCompound(makeVector()), srl(), srr(), sam(), prl(), pam(), rpd(), apd()
        {
            name = "I048/130 Radar Plot Characteristics (Placeholder)";
        }

    private:
        // Helper to build the vector using member addresses
        std::vector<AsterixDataItemHandlerBase*> makeVector() {
            return { &srl, &srr, &sam, &prl, &pam, &rpd, &apd };
        }
};

/**
 * @brief Handler for I048/220, Aircraft Address.
 * An optional, fixed-length (3-byte) item for the Mode S Aircraft Address.
 */
class I048_220_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 8;
        static constexpr bool mandatory = false;
        I048_220_Handler() : AsterixDataItemHandlerFixedLength(3) {
            name = "I048/220 Aircraft Address";
        }
};

/**
 * @brief Handler for I048/240, Aircraft Identification.
 * An optional, fixed-length (3-byte) item for the Mode S Aircraft Identification.
 */
class I048_240_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 9;
        static constexpr bool mandatory = false;
        I048_240_Handler() : AsterixDataItemHandlerFixedLength(6) {
            name = "I048/240 Aircraft Identification";
        }
};

/**
 * @brief Handler for I048/250, Mode S MB Data.
 * A repetitive, (8-byte) item for the Mode S MB Data
 */
class I048_250_Handler final : public AsterixDataItemHandlerRepetitive {
    public:
        static constexpr uint8_t FRN = 10;
        static constexpr bool mandatory = false;
        I048_250_Handler() : AsterixDataItemHandlerRepetitive(8) {
            name = "I048/250 Mode S MB Data";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I048/161, Track Number
 */
class I048_161_Handler final : public AsterixDataItemHandlerFixedLength {
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
class I048_042_Handler final : public AsterixDataItemHandlerFixedLength {
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
class I048_200_Handler final : public AsterixDataItemHandlerFixedLength {
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
class I048_170_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 14;
        static constexpr bool mandatory = false;
        I048_170_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I048/170, Track Status";
        }
};

/**
 * @brief Handler for I048/210, Track Quality.
 * An optional, fixed-length (4-byte) item.
 */
class I048_210_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 15;
        I048_210_Handler() : AsterixDataItemHandlerFixedLength(4) {
            name = "I048/210 Track Quality";
        }
};

/**
 * @brief Handler for I048/030, Warning/Error Conditions.
 * An optional, extended-length item.
 */
class I048_030_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 16;
        I048_030_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I048/030 Warning/Error Conditions";
        }
};

/**
 * @brief Handler for I048/080, Mode-3/A Code Confidence Indicator.
 * An optional, fixed-length (2-byte) item.
 */
class I048_080_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 17;
        I048_080_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/080 Mode-3/A Code Confidence Indicator";
        }
};

/**
 * @brief Handler for I048/100, Mode C Code and Confidence Indicator
 * An optional, fixed-length (4-byte) item.
 */
class I048_100_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 18;
        static constexpr bool mandatory = false;
        I048_100_Handler() : AsterixDataItemHandlerFixedLength(4) {
            name = "I048/100 Mode-C Code and Confidence Indicator";
        }
};

/**
 * @brief Handler for I048/110, Height measured by a 3D Radar.
 * An optional, fixed-length (2-byte) item.
 */
class I048_110_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 19;
        static constexpr bool mandatory = false;
        I048_110_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/110 Height measured by a 3D Radar";
        }

        /**
         * @brief Decodes the 2-byte Height from 3D Radar and converts it to a height in meters.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        void decode(std::string_view data) override;

        int16_t height;
};

/**
 * @brief Handler for I048/120, Radial Doppler Speed.
 * An optional, compound item.
 */
class I048_120_Handler final : public AsterixDataItemHandlerCompound {
    public:
        class CAL final : public AsterixDataItemHandlerFixedLength {
            public:
                CAL() : AsterixDataItemHandlerFixedLength(2) {
                    name = "Calculated Doppler Speed";
                }
        };

        class RDS final : public AsterixDataItemHandlerRepetitive {
            public:
                RDS() : AsterixDataItemHandlerRepetitive(6) {
                    name = "Raw Doppler Speed";
                }
        };

        CAL cal;
        RDS rds;

        I048_120_Handler()
            : AsterixDataItemHandlerCompound(makeVector()), cal(), rds()
        {
            name = "I048/120 Radial Doppler Speed";
        }

    private:
        // Helper to build the vector using member addresses
        std::vector<AsterixDataItemHandlerBase*> makeVector() {
            return { &cal, &rds };
        }
};


/**
 * @brief Handler for I048/230, Communication/ACAS Capability and Flight Status
 * An optional, fixed-length (2-byte) item.
 */
class I048_230_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 21;
        static constexpr bool mandatory = false;
        I048_230_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/230 Communication/ACAS Capability and Flight Status";
        }
};

/**
 * @brief Handler for I048/260, ACAS Resolution Advisory Report.
 * An optional, fixed-length (7-byte) item.
 */
class I048_260_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 22;
        I048_260_Handler() : AsterixDataItemHandlerFixedLength(7) {
            name = "I048/260 ACAS Resolution Advisory Report";
        }
};

/**
 * @brief Handler for I048/055, Mode-1 Code in Octal Representation.
 * An optional, fixed-length (1-byte) item (usually for military identification).
 */
class I048_055_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 23;
        I048_055_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I048/055 Mode-1 Code in Octal Representation";
        }
};

/**
 * @brief Handler for I048/050, Mode-2 Code in Octal Representation.
 * An optional, fixed-length (2-byte) item (usually for military identification).
 */
class I048_050_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 24;
        I048_050_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/050 Mode-2 Code in Octal Representation";
        }
};

/**
 *  * @brief Handler for I048/065, Mode-1 Code Confidence Indicator.
 *   * An optional, fixed-length (1-byte) item.
 *    */
class I048_065_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 25;
        I048_065_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I048/065 Mode-1 Code Confidence Indicator";
        }
};

/**
 *  * @brief Handler for I048/060, Mode-2 Code Confidence Indicator.
 *   * An optional, fixed-length (2-byte) item.
 *    */
class I048_060_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 26;
        I048_060_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/060 Mode-2 Code Confidence Indicator";
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
