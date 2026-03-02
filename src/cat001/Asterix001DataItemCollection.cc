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
#include <ReactorAsterix/cat001/Asterix001DataItemCollection.h>

// System headers
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

// Library headers
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/core/EndianUtils.h>
#include <ReactorAsterix/core/FastBitReader.h>
#include <ReactorAsterix/cat001/Asterix001Report.h>

namespace ReactorAsterix {

/**
 * @brief Handler for ASTERIX Data Item I001/010, Data Source Identifier.
 *
 * This item provides the System Area Code (SAC) and System Identification Code (SIC)
 * to uniquely identify the data source (e.g., the radar station).
 *
 * The first byte is the SAC, and the second byte is the SIC.
 *
 * @param data The raw data buffer for this item (2 bytes).
 */
size_t I001_010_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // data[0] is the System Area Code (SAC), and data[1] is the System Identification Code (SIC).
    // Explicit cast to avoid sign-conversion warnings
    sac = static_cast<uint8_t>(data[0]);
    sic = static_cast<uint8_t>(data[1]);

    return AsterixDataItemHandlerFixedLength::decode(data);
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
 * @param data The raw data buffer for this item.
 */
void I001_020_Handler::decodePrimary(std::string_view data) {
    const uint8_t octet = static_cast<uint8_t>(data[0]);

    typ = (octet >> 7) & 0x01;

    // Decode the SIM bit (Simulation - bits 6).
    sim = (octet >> 6) & 0x01;

    // Decode the 2 bits of the SSR/PSR (Target Report Type - bits 5-4).
    ssrpsr = static_cast<SSRPSR_T>((octet >> 4) & 0x03);

    ant = (octet >> 3) & 0x01;

    // Decode the SPI bit (Special Position Identification - bit 2).
    spi = (octet >> 2) & 0x01;

    // Decode the RAB bit (Report from Aircraft Transponder - bit 2).
    rab = (octet >> 1) & 0x01;
}

void I001_020_Handler::decodeExtension(uint32_t index, std::string_view data) {
    if (index == 1) { // First extension
        const uint8_t octet = static_cast<uint8_t>(data[0]);

        // Decode TST bit (Test, bit 8 2nd byte)
        tst = (octet >> 7) & 0x01;

        // Decode the 2 bits of the EMG (Emergency) subfield (bits 5-4).
        ds1ds2 = static_cast<DS1DS2_T>((octet >> 5) & 0x03);

        // Bit 5: ME (Military Emergency)
        me = (octet >> 4) & 0x01;

        // Bit 4: MI (Military Identification)
        mi = (octet >> 3) & 0x01;
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
 * @param data The raw data buffer for this item (4 bytes).
 */
size_t I001_040_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize)
        return 0;

    range   = readBigEndian<uint16_t>(data.data());
    azimuth = readBigEndian<uint16_t>(data.data() + 2);

    return AsterixDataItemHandlerFixedLength::decode(data);
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
 * @param data The raw data buffer for this item (2 bytes).
 */
size_t I001_070_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    // Check for presence/validity: bits 15, 14, and 13 must be zero (0xe000 mask).
    validated = !reader.readBit(bit);
    garbled   = reader.readBit(bit);
    local     = reader.readBit(bit);

    auto mode3ATemp = readBigEndian<uint16_t>(data.data());

    // Extract the 12 bits of the Mode 3/A code (0x0fff mask).
    code = mode3ATemp & 0x0FFF;

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for ASTERIX Data Item I001/090, Mode-C Code (Flight Level).
 *
 * Contains the Mode-C code, which represents the flight level (altitude) of the target.
 *
 * The 14-bit value is scaled by 25 ft and converted to meters.
 *
 * @param data The raw data buffer for this item (2 bytes).
 */
size_t I001_090_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    validated = !reader.readBit(bit);
    garbled = reader.readBit(bit);

    auto flightLevelTemp = readBigEndian<uint16_t>(data.data());

    // Clear the reserved bits and extract the 14-bit value.
    flightLevelTemp &= 0x3FFF;

    // The Mode-C value is a signed 14-bit integer, so perform sign extension.
    // If the MSB of the 14-bit value (bit 13, mask 0x2000) is set,
    // we set the upper bits (15 and 14) to 1 to complete the 16-bit sign extension.
    if (flightLevelTemp & 0x2000) {
        flightLevelTemp |= 0xC000;
    }

    height = static_cast<int16_t>(flightLevelTemp);

    return AsterixDataItemHandlerFixedLength::decode(data);
}

// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I001/141, Truncated Time of Day.
 *
 * A 2-byte item representing the time of day, truncated to a fixed resolution.
 *
 * Combines the two big-endian bytes into a single 16-bit value.
 *
 * @param report The target `Asterix001` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
size_t I001_141_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    todLSP = readBigEndian<uint16_t>(data.data());

    return AsterixDataItemHandlerFixedLength::decode(data);
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
