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

// Interface
#include "ReactorAsterix/cat001/Asterix1DataItemCollection.h"

// System headers
#include <arpa/inet.h>
#include <cstring>

// Library headers
#include "ReactorAsterix/cat001/Asterix1Report.h"
#include "ReactorAsterix/core/asterixExceptions.h"

namespace ReactorAsterix {

/**
 * @brief Handler for ASTERIX Data Item I001/010, Data Source Identifier.
 *
 * This item provides the System Area Code (SAC) and System Identification Code (SIC)
 * to uniquely identify the data source (e.g., the radar station).
 *
 * The first byte is the SAC, and the second byte is the SIC.
 *
 * @param report The target `Asterix1Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I001_010_Handler::decode(Asterix1Report& report, std::string_view data) const {
    // data[0] is the System Area Code (SAC), and data[1] is the System Identification Code (SIC).
    // Explicit cast to avoid sign-conversion warnings
    uint8_t sac = static_cast<uint8_t>(data[0]);
    uint8_t sic = static_cast<uint8_t>(data[1]);
    report.setSourceIdentifier(sac, sic);
}

// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I001/020, Target Report Descriptor.
 *
 * This extended length item provides information about the type and status of the report.
 * It contains subfields for Report Type, Special Position Identification (SPI), and Emergency status.
 *
 * The first byte is the main TRD. Further bytes extend the information if the FX bit is set.
 *
 * @param report The target `Asterix1Report` object.
 * @param data The raw data buffer for this item.
 */
void I001_020_Handler::decode(Asterix1Report& report, std::string_view data) const {
    const auto octet1 = data[0];

    // Check for uninterpreted reserved bits in the first octet (bits 7, and 6).
    constexpr uint8_t RESERVED_BITS_OCTET1 = 0xC0;
    if (octet1 & RESERVED_BITS_OCTET1) {
        throw uninterpretedItem();
    }

    // Decode the 2 bits of the SSR/PSR (Target Report Type - bits 5-4).
    const uint8_t ssr_psr = (octet1 & 0x30) >> 4;
    report.setSSR_PSR(ssr_psr);

    // Decode the SPI bit (Special Position Identification - bit 2).
    if (octet1 & 0x04) {
        report.setSPI(true);
    }

    // Check the FX bit (bit 0) to see if the second octet exists.
    if (octet1 & 0x01) {
        const auto octet2 = static_cast<uint8_t>(data[1]);

        // Check for uninterpreted reserved bits in the second octet (bits 7, 4, and 3).
        constexpr uint8_t RESERVED_BITS_OCTET2 = 0x98;
        if (octet2 & RESERVED_BITS_OCTET2) {
            throw uninterpretedItem();
        }

        // Decode the 2 bits of the EMG (Emergency) subfield (bits 5-4).
        const uint8_t ds1ds2 = (octet2 & 0x60) >> 5;
        report.setDs1Ds2(ds1ds2);

        // Check the FX bit (bit 0) of the second octet for the third octet.
        if (octet2 & 0x01) {
            throw uninterpretedItem();
        }
    }
}

// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I001/040, Measured Position in Polar Coordinates.
 *
 * This item contains the target's measured range and azimuth from the radar site.
 *
 * The data is 4 bytes: 2 for range and 2 for azimuth, both in big-endian format.
 * Range is scaled by $(1/128) \text{ NM}$. Azimuth is scaled by $(\pi/4) / 8192 \text{ radians}$.
 *
 * @param report The target `Asterix1Report` object.
 * @param data The raw data buffer for this item (4 bytes).
 */
void I001_040_Handler::decode(Asterix1Report& report, std::string_view data) const {
    uint16_t rawRange;
    uint16_t rawAzimuth;

    // Use memcpy for type-punning, a safer alternative to C-style casts.
    // The data is big-endian (network byte order).
    std::memcpy(&rawRange, data.data(), 2);
    std::memcpy(&rawAzimuth, data.data() + 2, 2);

    rawRange   = ntohs(rawRange);
    rawAzimuth = ntohs(rawAzimuth);

    // Range: LSB = 1/128 NM converted to meters
    // 1852.0 is the standard Nautical Mile to Meters conversion
    report.range = (static_cast<double>(rawRange) / 128.0) * 1852.0;

    // Azimuth: LSB = (pi/4) / 8192 radians
    // Which is 2*PI / 65536
    constexpr double AZIMUTH_SCALE = 0.00009587379; // (M_PI / 32768.0)
    report.azimuth = static_cast<double>(rawAzimuth) * AZIMUTH_SCALE;
}

// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I001/070, Mode-3/A Code.
 *
 * Contains the Mode-3/A code (squawk) in octal representation, typically used for
 * aircraft identification.
 *
 * The code is present if the top three bits (15, 14, 13) are all zero.
 * The 12-bit code is then extracted.
 *
 * @param report The target `Asterix1Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I001_070_Handler::decode(Asterix1Report& report, std::string_view data) const {
    uint16_t mode3ATemp;
    std::memcpy(&mode3ATemp, data.data(), 2);
    mode3ATemp = ntohs(mode3ATemp);

    // Check for presence/validity: bits 15, 14, and 13 must be zero (0xe000 mask).
    const bool validity = mode3ATemp & 0x8000;
    const bool garbled  = mode3ATemp & 0x4000;
    const bool local    = mode3ATemp & 0x2000;
    // Extract the 12 bits of the Mode 3/A code (0x0fff mask).
    uint16_t mode3A = mode3ATemp & 0x0FFF;
    report.setMode3A(mode3A, validity, garbled, local);
}

/**
 * @brief Handler for ASTERIX Data Item I001/090, Mode-C Code (Flight Level).
 *
 * Contains the Mode-C code, which represents the flight level (altitude) of the target.
 *
 * The code is present if the top two bits (15, 14) are zero.
 * The 14-bit value is scaled by $(1/4) \text{ NM}$ and converted to meters.
 *
 * @param report The target `Asterix1Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I001_090_Handler::decode(Asterix1Report& report, std::string_view data) const {
    uint16_t flightLevelTemp;
    std::memcpy(&flightLevelTemp, data.data(), 2);
    flightLevelTemp = ntohs(flightLevelTemp);

    bool v = flightLevelTemp & 0x8000;
    bool g = flightLevelTemp & 0x4000;

    // Clear the reserved bits and extract the 14-bit value.
    flightLevelTemp &= 0x3FFF;

    // The Mode-C value is a signed 14-bit integer, so perform sign extension.
    // If the MSB of the 14-bit value (bit 13, mask 0x2000) is set,
    // we set the upper bits (15 and 14) to 1 to complete the 16-bit sign extension.
    if (flightLevelTemp & 0x2000) {
        flightLevelTemp |= 0xC000;
    }

    int16_t flValue = static_cast<int16_t>(flightLevelTemp);

    // The resolution is 1/4 NM (Nautical Miles). The scale factor converts
    // the signed 16-bit value (interpreted as a flight level) to meters.
    // Resolution is $25 \text{ feet}$ * $(1/4)$ units $\times \text{FOOT\_TO\_METER}$.
    constexpr double HEIGHT_SCALE = 25.0 * 0.3048;

    // Cast to int16_t to correctly interpret the signed value after sign extension.
    double height = flValue * HEIGHT_SCALE;

    report.setSSRHeight(height, v, g);
}

// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I001/141, Truncated Time of Day.
 *
 * A 2-byte item representing the time of day, truncated to a fixed resolution.
 *
 * Combines the two big-endian bytes into a single 16-bit value.
 *
 * @param report The target `Asterix1Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I001_141_Handler::decode(Asterix1Report& report, std::string_view data) const {
    report.todLSP = (static_cast<uint16_t>(data[0]) << 8) | data[1];
    report.hasLspClock = true;
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
