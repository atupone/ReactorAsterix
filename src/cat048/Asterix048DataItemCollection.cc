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
#include <ReactorAsterix/cat048/Asterix048DataItemCollection.h>


// Library headers
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/core/EndianUtils.h>
#include <ReactorAsterix/core/FastBitReader.h>
#include <ReactorAsterix/cat048/Asterix048Report.h>

namespace ReactorAsterix {

/**
 * @brief Handler for ASTERIX Data Item I048/010, Data Source Identifier.
 *
 * This item provides the System Area Code (SAC) and System Identification Code (SIC)
 * to uniquely identify the data source (e.g., the radar station).
 *
 * The first byte is the SAC, and the second byte is the SIC.
 *
 * @param report The target `Asterix048Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I048_010_Handler::decode(Asterix048Report& report, std::string_view data) const {
    // data[0] is the System Area Code (SAC), and data[1] is the System Identification Code (SIC).
    // Explicit cast to avoid sign-conversion warnings
    uint8_t sac = static_cast<uint8_t>(data[0]);
    uint8_t sic = static_cast<uint8_t>(data[1]);
    report.setSourceIdentifier(sac, sic);
}

// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I048/020, Target Report Descriptor.
 *
 * This extended length item provides information about the type and status of the report.
 * It contains subfields for Report Type, Special Position Identification (SPI), and Emergency status.
 *
 * The first byte is the main TRD. Further bytes extend the information if the FX bit is set.
 *
 * @param report The target `Asterix048Report` object.
 * @param data The raw data buffer for this item.
 */
void I048_020_Handler::decode(Asterix048Report& report, std::string_view data) const {
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    // Decode the 3 bits of the TYP (bits 8, 7 and 6).
    uint8_t typ = reader.readBits<3>(bit);
    report.setTYP(typ);

    // Decode the SIM bit (Simulation - bits 5).
    const uint8_t sim = reader.readBit(bit);
    if (sim) {
        stats_ptr->uninterpretedItems.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    // skip the RDP bit (Radar Display Processor Chain - bits 4).
    reader.skipBits(bit, 1);

    // Decode the SPI bit (Special Position Identification - bit 3).
    bool spi = reader.readBit(bit);
    report.setSPI(spi);

    // Decode the RAB bit (Report from Aircraft Transponder - bit 2).
    bool rab = reader.readBit(bit);
    if (rab) {
        stats_ptr->uninterpretedItems.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    // Check the FX bit (bit 0) to see if the second octet exists.
    bool fx = reader.readBit(bit);
    if (fx) {
        // Decode TST bit (Test, bit 8 2nd byte)
        bool tst = reader.readBit(bit);
        if (tst) {
            stats_ptr->uninterpretedItems.fetch_add(1, std::memory_order_relaxed);
            return;
        }

        // skip the ERR and XPP bit (Extended Range, X-Pulse - bits 7-6).
        reader.skipBits(bit, 2);

        bool me = reader.readBit(bit);
        report.setME(me);
    }
}

// ----------------------------------------------------------------------------------

/**
 * @brief Decodes the 3-byte Time of Day (TOD).
 * The TOD value is constructed from the three bytes, where the unit is in
 * 1/128 seconds.
 *
 * @param context The target context object (Asterix048Report) to store the result.
 * @param data The raw data buffer containing the 3 bytes of TOD.
 */
void I048_140_Handler::decode(Asterix048Report& context, std::string_view data) const {
    if (data.size() < fixedSize) [[unlikely]] {
        return;
    }

    context.TOD = decodeBigEndian<uint32_t>(data.substr(0, 3));
}

// ----------------------------------------------------------------------------------
/**
 * @brief Handler for ASTERIX Data Item I048/040, Measured Position in Polar Coordinates.
 *
 * This item contains the target's measured range and azimuth from the radar site.
 *
 * The data is 4 bytes: 2 for range and 2 for azimuth, both in big-endian format.
 * Range is scaled by 1/256 NM. Azimuth is scaled by pi/4 / 8192 radians.
 *
 * @param report The target `Asterix48Report` object.
 * @param data The raw data buffer for this item (4 bytes).
 */
void I048_040_Handler::decode(Asterix048Report& report, std::string_view data) const {
    auto rawRange   = readBigEndian<uint16_t>(data.data());
    auto rawAzimuth = readBigEndian<uint16_t>(data.data() + 2);

    report.setMeasuredCoordinates(rawRange, rawAzimuth);
}

// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I048/070, Mode-3/A Code.
 *
 * Contains the Mode-3/A code (squawk) in octal representation, typically used for
 * aircraft identification.
 *
 * The code is present if the top three bits (15, 14, 13) are all zero.
 * The 12-bit code is then extracted.
 *
 * @param report The target `Asterix048Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I048_070_Handler::decode(Asterix048Report& report, std::string_view data) const {
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    // Check for presence/validity: bits 15, 14, and 13 must be zero (0xe000 mask).
    const bool validated = !reader.readBit(bit);
    const bool garbled   = reader.readBit(bit);
    const bool local     = reader.readBit(bit);

    auto mode3ATemp = readBigEndian<uint16_t>(data.data());

    // Extract the 12 bits of the Mode 3/A code (0x0fff mask).
    uint16_t mode3A = mode3ATemp & 0x0FFF;
    report.setMode3A(mode3A, validated, garbled, local);
}

/**
 * @brief Handler for ASTERIX Data Item I048/090, Flight Level in Binary Representation.
 *
 * Contains the Mode-C code, which represents the flight level (altitude) of the target.
 *
 * The 14-bit value is scaled by 25 ft and converted to meters.
 *
 * @param report The target `Asterix048Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I048_090_Handler::decode(Asterix048Report& report, std::string_view data) const {
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    const bool v = !reader.readBit(bit);
    const bool g = reader.readBit(bit);

    auto flightLevelTemp = readBigEndian<uint16_t>(data.data());

    // Clear the reserved bits and extract the 14-bit value.
    flightLevelTemp &= 0x3FFF;

    // The Mode-C value is a signed 14-bit integer, so perform sign extension.
    // If the MSB of the 14-bit value (bit 13, mask 0x2000) is set,
    // we set the upper bits (15 and 14) to 1 to complete the 16-bit sign extension.
    if (flightLevelTemp & 0x2000) {
        flightLevelTemp |= 0xC000;
    }

    int16_t flValue = static_cast<int16_t>(flightLevelTemp);

    report.setSSRHeight(flValue, v, g);
}

/**
 * @brief Handler for ASTERIX Data Item I048/110, Height from a 3D-Radar
 *
 * Contains the height (altitude) of the target is 25ft.
 *
 * @param report The target `Asterix048Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I048_110_Handler::decode(Asterix048Report& report, std::string_view data) const {
    auto flightLevelTemp = readBigEndian<uint16_t>(data.data());

    // Clear the reserved bits and extract the 14-bit value.
    flightLevelTemp &= 0x3FFF;

    // The height value is a signed 14-bit integer, so perform sign extension.
    // If the MSB of the 14-bit value (bit 13, mask 0x2000) is set,
    // we set the upper bits (15 and 14) to 1 to complete the 16-bit sign extension.
    if (flightLevelTemp & 0x2000) {
        flightLevelTemp |= 0xC000;
    }

    int16_t flValue = static_cast<int16_t>(flightLevelTemp);

    report.setSRHeight(flValue);
}

// ----------------------------------------------------------------------------------





} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
