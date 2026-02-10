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
#include <ReactorAsterix/core/AsterixDataItemHandlerRepetitive.h>
#include <ReactorAsterix/core/AsterixDataItemHandlerCompound.h>

// System headers
#include <cstdlib>

namespace ReactorAsterix {

// The context object
class Asterix034Report;

/**
 * @file Asterix034DataItemCollection.h
 * @brief Declares the specific handler classes for **ASTERIX Category 034** data items.
 *
 * Each class publicly inherits from an appropriate base class (`AsterixDataItemHandlerFixedLength`
 * or `AsterixDataItemHandlerExtendedLength`) and is responsible for decoding a single
 * data item into the `Asterix1Report` context object.
 */

// ----------------------------------------------------------------------------------
// ASTERIX CAT 034 DATA ITEM HANDLERS (Standard v1.29)
// ----------------------------------------------------------------------------------

/**
 * @brief Handler for I048/010, Data Source Identifier.
 * A mandatory, fixed-length (2-byte) item providing the SAC and SIC.
 */
class I034_010_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 1;
        static constexpr bool mandatory = true;
        I034_010_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I034/010 Data Source Identifier";
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
 * @brief Handler for ASTERIX Data Item I034/000, Message Type.
 *
 * This mandatory, 1-byte item identifies the type of message being transmitted.
 */
class I034_000_Handler final: public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 2;
        static constexpr bool mandatory = true;
        I034_000_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I034/000, Message Type";
        }

        void decode(std::string_view data) override;

        enum class MESSAGE_TYPE_T : uint8_t {
            NORTH_MARKER = 1,
            SECTOR_CROSSING = 2,
            GEOGRAPHICAL_FILTER = 3,
            JAMMING_STROBE = 4,
            SOLAR_STORM = 5,
            SSR_JAMMING_STROBE = 6,
            MODE_S_JAMMING_STROBE = 7
        };

        MESSAGE_TYPE_T messageType{MESSAGE_TYPE_T::NORTH_MARKER};
};

/**
 * @brief Handler for ASTERIX Data Item I034/030, Time of Day.
 *
 * This mandatory, 3-byte item represents the time of day, typically as the
 * number of seconds since midnight, in 1/128 second increments.
 */
class I034_030_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 3;
        static constexpr bool mandatory = false;
        I034_030_Handler(): AsterixDataItemHandlerFixedLength(3) {
            name = "I034/030, Time of Day";
        }
        void decode(std::string_view data) override;

        uint32_t TOD{0};
};

/**
 * @brief Handler for ASTERIX Data Item I034/020, Sector Number.
 *
 * This optional, 1-byte item specifies the sector number from which the data
 * originated, typically used in multi-sector radar systems.
 */
class I034_020_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 4;
        I034_020_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I034/020, Sector Number";
        }
        void decode(std::string_view data) override;

        uint8_t sectorNumber{0};
};

class I034_041_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 5; // Standard FRN for Rotation Period
        I034_041_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I034/041, Antenna Rotation Period";
        }
        void decode(std::string_view data) override;

        uint16_t speed{0};
};

/**
 * @brief I034/050 System Configuration and Status (Compound Item)
 */
class I034_050_Handler final : public AsterixDataItemHandlerCompound {
    public:
        static constexpr uint8_t FRN = 6;

        class COM final : public AsterixDataItemHandlerFixedLength {
            public:
                COM() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Common Part";
                }
        };

        class PSR final : public AsterixDataItemHandlerFixedLength {
            public:
                PSR() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Specific Status for PSR Sensor";
                }
        };

        class SSR final : public AsterixDataItemHandlerFixedLength {
            public:
                SSR() : AsterixDataItemHandlerFixedLength(1) {
                    name = "Specific Status for SSR Sensor";
                }
        };

        class MDS final : public AsterixDataItemHandlerFixedLength {
            public:
                MDS() : AsterixDataItemHandlerFixedLength(2) {
                    name = "Specific Status for Mode S Sensor";
                }
        };

        COM com;
        PSR psr;
        SSR ssr;
        MDS mds;

        I034_050_Handler()
            : AsterixDataItemHandlerCompound(makeVector()), com(), psr(), ssr(), mds()
        {
            name = "I034/050 System Configuration and Status";
        }

    private:
        // Helper to build the vector using member addresses
        std::vector<AsterixDataItemHandlerBase*> makeVector() {
            return { &com, nullptr, nullptr, &psr, &ssr, &mds};
        }
};

/**
 * @brief I034/060 System Processing Parameters (Compound item)
 */
class I034_060_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 7;
        I034_060_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I034/060 System Processing Parameters";
        }
        void decode(std::string_view /* data */) override {
            std::abort();
        }
};

/**
 * @brief I034/070 Plot Count Values (Repetitive)
 */
class I034_070_Handler final : public AsterixDataItemHandlerRepetitive {
    public:
        static constexpr uint8_t FRN = 8;
        I034_070_Handler() : AsterixDataItemHandlerRepetitive(2) {
            name = "I034/070 Plot Count Values";
        }
};

/**
 * @brief I034/100 Generic Polar Window (8 bytes)
 */
class I034_100_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 9;
        I034_100_Handler() : AsterixDataItemHandlerFixedLength(8) {
            name = "I034/100 Generic Polar Window";
        }
};

/**
 * @brief I034/110 Data Filter (1 byte)
 */
class I034_110_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 10;
        I034_110_Handler() : AsterixDataItemHandlerFixedLength(1) {
            name = "I034/110 Data Filter";
        }
};

/**
 * @brief Handler for ASTERIX Data Item I034/120, 3D-Position of Data Source
 *
 * This optional, 8-byte item specifies the location of the Radar
 */
class I034_120_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 11;
        I034_120_Handler() : AsterixDataItemHandlerFixedLength(8) {
            name = "I034/120, 3D-Position of Data Source";
        }
        void decode(std::string_view data) override;

        uint16_t height{0};
        int32_t  latitude{0};
        int32_t  longitude{0};
};

/**
 * @brief I034/110 Data Filter (1 byte)
 */
class I034_090_Handler final : public AsterixDataItemHandlerFixedLength {
    public:
        static constexpr uint8_t FRN = 12;
        I034_090_Handler() : AsterixDataItemHandlerFixedLength(2) {
            name = "I034/090 Collimation Error";
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
