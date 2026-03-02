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
#include <string>

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
        I048_010_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/010 Data Source Identifier";
            mandatory = true;
        }

        /**
         * @brief Decodes the 2-byte Data Source Identifier.
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t decode(std::string_view data) override;

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
        I048_140_Handler(): AsterixDataItemHandlerFixedLength(3) {
            name = "I048/140, Time of Day";
            mandatory = true;
        }

        [[nodiscard]] size_t decode(std::string_view data) override;

        uint32_t TOD;
};

/**
 * @brief Handler for I048/020, Target Report Descriptor.
 * A mandatory, extended-length item describing the nature and status of the report.
 */
class I048_020_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 3;
        I048_020_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I048/020 Target Report Descriptor";
            mandatory = true;
        }
        /**
         * @brief Decodes the Target Report Descriptor (TRT, SPI, EMG, etc.).
         * @param data The raw data buffer.
         */
        void decodePrimary(std::string_view data) override;
        void decodeExtension(uint32_t index, std::string_view data) override;

        inline void reset() {
            AsterixDataItemHandlerExtendedLength::reset();
            typ = TYP_T::NO_DETECTION;
            sim = false;
            spi = false;
            rab = false;
            tst = false;
            me  = false;
        };

        // Enumeration for TYP
        enum class TYP_T : uint8_t {
            NO_DETECTION           = 0,
            SINGLE_PSR_DETECTION   = 1,
            SINGLE_SSR_DETECTION   = 2,
            SSR_PSR_DETECTION      = 3,
            SINGLE_MODES_ALL_CALL  = 4,
            SINGLE_MODES_ROLL_CALL = 5,
            MODES_ALL_CALL_PSR     = 6,
            MODES_ROLL_CALL_PSR    = 7
        };

        // Enumeration for FOE/FRI
        enum class FOE_FRI_T : uint8_t {
            NO_MODE4_INTERROGATION = 0,
            FRIENDLY_TARGET        = 1,
            UNKNOWN_TARGET         = 2,
            NO_REPLY               = 3
        };

        struct EP_VAL_T {
            bool ep;
            uint8_t val;
        };

        TYP_T typ{TYP_T::NO_DETECTION};
        bool  sim{false};
        bool  rdp{false};
        bool  spi{false};
        bool  rab{false};
        bool  tst{false};
        bool  err{false};
        bool  xpp{false};
        bool  me{false};
        bool  mi{false};
        FOE_FRI_T foe_fri{FOE_FRI_T::NO_MODE4_INTERROGATION};
        EP_VAL_T adsb{false, 0};
        EP_VAL_T scn{false, 0};
        EP_VAL_T pai{false, 0};
        EP_VAL_T acasxv{false, 0};
        EP_VAL_T poxpr{false, 0};
        EP_VAL_T poact{false, 0};
        EP_VAL_T dtfxpr{false, 0};
        EP_VAL_T dtfact{false, 0};
        EP_VAL_T irmpr{false, 0};
        EP_VAL_T irmact{false, 0};
};

/**
 * @brief Handler for I048/040, Measured Position in Polar Coordinates.
 * An optional, fixed-length (4-byte) item for Range and Azimuth.
 */
class I048_040_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 4;
        I048_040_Handler() : AsterixDataItemHandlerFixedLength(4) {
            name = "I048/040 Measured Position in Polar Coordinates";
        };

        /**
         * @brief Decodes the 4-byte measured range and azimuth.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t decode(std::string_view data) override;

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
        [[nodiscard]] size_t decode(std::string_view data) override;

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
        [[nodiscard]] size_t decode(std::string_view data) override;

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

        I048_130_Handler();

        class SRL final : public AsterixDataItemHandlerFixedLength {
            public:
                SRL() : AsterixDataItemHandlerFixedLength(1) {
                    name = "SSR Plot Runlength";
                }

                [[nodiscard]] size_t decode(std::string_view data) override;

                uint8_t ssrRunlength{0};
        };

        class SRR final : public AsterixDataItemHandlerFixedLength {
            public:
                SRR() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Number of Received Replies for (M)SSR";
                }

                [[nodiscard]] size_t decode(std::string_view data) override;

                uint8_t numSsrReplies{0};
        };

        class SAM final : public AsterixDataItemHandlerFixedLength {
            public:
                SAM() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Amplitude of (M)SSR reply";
                }

                [[nodiscard]] size_t decode(std::string_view data) override;

                int8_t ssrReplyAmplitude{0};
        };

        class PRL final : public AsterixDataItemHandlerFixedLength {
            public:
                PRL() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Primary Plot Runlength";
                }

                [[nodiscard]] size_t decode(std::string_view data) override;

                uint8_t psrRunlength{0};
        };

        class PAM final : public AsterixDataItemHandlerFixedLength {
            public:
                PAM() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Amplitude of Primary Plot";
                }

                [[nodiscard]] size_t decode(std::string_view data) override;

                int8_t psrReplyAmplitude{0};
        };

        class RPD final : public AsterixDataItemHandlerFixedLength {
            public:
                RPD() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Difference in Range between PSR and SSR plot";
                }

                [[nodiscard]] size_t decode(std::string_view data) override;

                int8_t rangeDifference{0};
        };

        class APD final : public AsterixDataItemHandlerFixedLength {
            public:
                APD() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Difference in Azimuth between PSR and SSR plot";
                }

                [[nodiscard]] size_t decode(std::string_view data) override;

                int8_t azimuthDifference{0};
        };

        SRL srl;
        SRR srr;
        SAM sam;
        PRL prl;
        PAM pam;
        RPD rpd;
        APD apd;
};

/**
 * @brief Handler for I048/220, Aircraft Address.
 * An optional, fixed-length (3-byte) item for the Mode S Aircraft Address.
 */
class I048_220_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 8;
        I048_220_Handler() : AsterixDataItemHandlerFixedLength(3) {
            name = "I048/220 Aircraft Address";
        }

        [[nodiscard]] size_t decode(std::string_view data) override;

        uint32_t address; // 24-bit address
};

/**
 * @brief Handler for I048/240, Aircraft Identification.
 * An optional, fixed-length (3-byte) item for the Mode S Aircraft Identification.
 */
class I048_240_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 9;
        I048_240_Handler() : AsterixDataItemHandlerFixedLength(6) {
            name = "I048/240 Aircraft Identification";
        }

        [[nodiscard]] size_t decode(std::string_view data) override;

        std::string identification;
};

/**
 * @brief Sub-structure for I048/250 Mode S MB Data
 * Each repetition is an 8-byte (64-bit) BDS register.
 */
struct ModeS_MB_Entry {
    static constexpr size_t FixedLength = 8;
    uint64_t msg{0};

    void decode(std::string_view data) {
        // Reads 8 bytes as a single 64-bit word
        msg = readBigEndian<uint64_t>(data.data());
    }
};

/**
 * @brief Handler for I048/250, Mode S MB Data.
 * A repetitive, (8-byte) item for the Mode S MB Data
 */
class I048_250_Handler final : public AsterixDataItemHandlerRepetitive<ModeS_MB_Entry> {
    public:
        static constexpr uint8_t FRN = 10;
        I048_250_Handler() {
            name = "I048/250 Mode S MB Data";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I048/161, Track Number
 */
class I048_161_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 11;
        I048_161_Handler(): AsterixDataItemHandlerFixedLength(2) {
            name = "I048/161, Track Number";
        }

        [[nodiscard]] size_t decode(std::string_view data) override;

        uint16_t trackNumber{0};
};

/**
 * @brief Handler for ASTERIX Data Item I048/042, Calculated Position in Cartesian Co-ordinates.
 */
class I048_042_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 12;
        I048_042_Handler(): AsterixDataItemHandlerFixedLength(4) {
            name = "I048/042, Calculated Position in Cartesian Co-ordinates";
        }

        [[nodiscard]] size_t decode(std::string_view data) override;

        int16_t x;
        int16_t y;
};

/**
 * @brief Handler for ASTERIX Data Item I048/200, Calculated Track Velocity in Polar Co-ordinates.
 */
class I048_200_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 13;
        I048_200_Handler(): AsterixDataItemHandlerFixedLength(4) {
            name = "I048/200, Calculated Track Velocity in Polar Co-ordinates";
        }

        [[nodiscard]] size_t decode(std::string_view data) override;

        uint16_t groundSpeed{0};
        uint16_t trackAngle{0};
};

/**
 * @brief Handler for ASTERIX Data Item I048/170, Track Status
 */
class I048_170_Handler final : public AsterixDataItemHandlerExtendedLength {
    public:
        static constexpr uint8_t FRN = 14;
        I048_170_Handler() : AsterixDataItemHandlerExtendedLength(1, 1) {
            name = "I048/170, Track Status";
        }

        void decodePrimary(std::string_view data) override;
        void decodeExtension(uint32_t index, std::string_view data) override;

        // Enumeration for RAD
        enum class RAD_T : uint8_t {
            COMBINED_TRACK   = 0,
            PSR_TRACK        = 1,
            SSR_MODE_S_TRACK = 2,
            INVALID          = 3
        };

        // Enumeration for CDM
        enum class CDM_T : uint8_t {
            MAINTAINING = 0,
            CLIMBING    = 1,
            DESCENDING  = 2,
            UNKNOWN     = 3
        };

        // Status Bits
        bool cnf{false};                  // Confirmed vs Tentative Track
        RAD_T rad{RAD_T::COMBINED_TRACK}; // Type of Sensor(s) maintaining Track
        bool dou{false};                  // Signals level of confidence in plot to track association process
        bool mah{false};                  // Manoeuvre detection in Horizontal Sense
        CDM_T cdm{CDM_T::MAINTAINING};    // Climbing / Descending Mode
        bool tre{false};                  // Signal for End_Of_Track
        bool gho{false};                  // Ghost vs. true target
        bool sup{false};                  // Track maintained with track information from network
        bool tcc{false};                  // Type of plot coordinate transformation mechanism
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

        enum class WarningCode : uint8_t {
            // --- Warning/Error Conditions ---
            MULTIPATH_REPLY                      = 1,
            SIDELOBE_REPLY                       = 2,
            SPLIT_PLOT                           = 3,
            NON_EXPECTED_REPLY                   = 4,
            ANGEL                                = 5,
            TERRESTRIAL_VEHICLE                  = 6,
            FIXED_PSR_PLOT                       = 7,
            SLOW_PSR_TARGET                      = 8,
            LOW_QUALITY_PSR_PLOT                 = 9,
            PHANTOM_SSR_PLOT                     = 10,
            NON_MATCHING_MODE_3A                 = 11,
            MODE_C_S_ALTITUDE_ABNORMAL           = 12,
            TARGET_IN_CLUTTER_AREA               = 13,
            MAX_DOPPLER_RESPONSE_ZERO_FILTER     = 14,
            TRANSPONDER_ANOMALY                  = 15,
            DUPLICATED_OR_ILLEGAL_MODE_S_ADDR    = 16,
            MODE_S_ERROR_CORRECTION_APPLIED      = 17,
            UNDECODABLE_MODE_C_S_ALTITUDE        = 18,
            BIRDS                                = 19,
            FLOCK_OF_BIRDS                       = 20,
            MODE_1_PRESENT_IN_REPLY              = 21,
            MODE_2_PRESENT_IN_REPLY              = 22,
            WIND_TURBINE_PLOT                    = 23,
            HELICOPTER                           = 24,
            MAX_REINTERROGATIONS_REACHED         = 25,
            MAX_REINTERROGATIONS_BDS_EXTRACTION  = 26,
            BDS_OVERLAY_INCOHERENCE              = 27,
            POTENTIAL_BDS_SWAP_DETECTED          = 28,
            TRACK_UPDATE_IN_ZENITHAL_GAP         = 29,
            MODE_S_TRACK_REACQUIRED              = 30,
            DUPLICATED_MODE_5_PAIR_NO_PIN        = 31,
            WRONG_DF_REPLY_FORMAT                = 32,
            TRANSPONDER_ANOMALY_ALL_CALL         = 33,
            TRANSPONDER_ANOMALY_SI_CAPABILITY    = 34,
            POTENTIAL_IC_CONFLICT                = 35,
            IC_CONFLICT_DETECTION_POSSIBLE       = 36,
            DUPLICATE_MODE_5_PIN                 = 37
        };

        std::vector<WarningCode> warningCodes;

        void decodePrimary(std::string_view data) override;
        void decodeExtension(uint32_t index, std::string_view data) override;

        inline void reset() {
            warningCodes.clear();
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

        [[nodiscard]] size_t decode(std::string_view data) override;

        uint16_t confidenceMask{0}; // 12 bits of confidence (0=High, 1=Low)
};

/**
 * @brief Handler for I048/100, Mode C Code and Confidence Indicator
 * An optional, fixed-length (4-byte) item.
 */
class I048_100_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 18;
        I048_100_Handler() : AsterixDataItemHandlerFixedLength(4) {
            name = "I048/100 Mode-C Code and Confidence Indicator";
        }

        [[nodiscard]] size_t decode(std::string_view data) override;

        bool validated{false};
        bool garbled{false};
        uint16_t grayCode{0};
        uint16_t confidence{0};
};

/**
 * @brief Handler for I048/110, Height measured by a 3D Radar.
 * An optional, fixed-length (2-byte) item.
 */
class I048_110_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 19;
        I048_110_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I048/110 Height measured by a 3D Radar";
        }

        /**
         * @brief Decodes the 2-byte Height from 3D Radar and converts it to a height in meters.
         * @param context The target `Asterix048Report` object.
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t decode(std::string_view data) override;

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

                [[nodiscard]] size_t decode(std::string_view data) override;

                bool    isDoubtful{false};
                int16_t speed{0};
        };

        struct DopplerEntry {
            static constexpr size_t FixedLength = 6;

            int16_t  dopplerSpeed{0};
            uint16_t ambiguityRange{0};
            uint16_t frequency{0};

            void decode(std::string_view data) {
                dopplerSpeed   = readBigEndian<int16_t>(data.data());
                ambiguityRange = readBigEndian<uint16_t>(data.data() + 2);
                frequency      = readBigEndian<uint16_t>(data.data() + 4);
            }
        };

        class RDS final : public AsterixDataItemHandlerRepetitive<DopplerEntry> {
            public:
                RDS() {
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

        [[nodiscard]] size_t decode(std::string_view data) override;

        bool validated{false};
        bool garbled{false};
        bool local{false};
        uint8_t code{0};
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

        /**
         * @brief Decodes the 2-byte Mode-2 code.
         * @param data The raw data buffer.
         */
        [[nodiscard]] size_t decode(std::string_view data) override;

        uint16_t code{0};
        bool validated{false};
        bool garbled{false};
        bool local{false};
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

        [[nodiscard]] size_t decode(std::string_view data) override;

        uint8_t confidenceMask{0}; // 5 bits of confidence indicators
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

        [[nodiscard]] size_t decode(std::string_view data) override;

        uint16_t confidenceMask{0}; // 12 bits of confidence indicators
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
