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
 * @param data The raw data buffer for this item (2 bytes).
 */
size_t I048_010_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // data[0] is the System Area Code (SAC), and data[1] is the System Identification Code (SIC).
    // Explicit cast to avoid sign-conversion warnings
    sac = static_cast<uint8_t>(data[0]);
    sic = static_cast<uint8_t>(data[1]);

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Decodes the 3-byte Time of Day (TOD).
 * The TOD value is constructed from the three bytes, where the unit is in
 * 1/128 seconds.
 *
 * @param data The raw data buffer containing the 3 bytes of TOD.
 */
size_t I048_140_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // Use a pointer to unsigned to avoid messy casting
    auto* udata = reinterpret_cast<const uint8_t*>(data.data());

    TOD =
        (static_cast<uint32_t>(udata[0]) << 16) |
        (static_cast<uint32_t>(udata[1]) << 8)  |
        (static_cast<uint32_t>(udata[2]));

    return AsterixDataItemHandlerFixedLength::decode(data);
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
 * @param data The raw data buffer for this item.
 */
size_t I048_020_Handler::decode(std::string_view data) {
    if (data.empty()) return 0;

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB

    // Decode the 3 bits of the TYP (bits 8, 7 and 6).
    typ = static_cast<TYP_T>(reader.readBits<3>(bit));

    // Decode the SIM bit (Simulation - bits 5).
    sim = reader.readBit(bit);

    // Decode the RDP bit (Radar Display Processor Chain - bits 4).
    rdp = reader.readBit(bit);

    // Decode the SPI bit (Special Position Identification - bit 3).
    spi = reader.readBit(bit);

    // Decode the RAB bit (Report from Aircraft Transponder - bit 2).
    rab = reader.readBit(bit);

    // Check the FX bit (bit 0) to see if the second octet exists.
    bool fx = reader.readBit(bit);

    size_t consumed = 1;

    if (fx) {
        // Safe Check: Can we read the 2nd byte?
        if (consumed >= data.size()) return 0;

        // Decode TST bit (Test, bit 8 2nd byte)
        tst = reader.readBit(bit);

        // decode ERR bit (Extended Range, bits 7).
        err = reader.readBit(bit);

        // decode XPP bit (X-Pulse - bits 6).
        xpp = reader.readBit(bit);

        me = reader.readBit(bit); // Bit 5: Military Emergency

        mi = reader.readBit(bit); // Bit 4: Military Identification

        // Bits 3-2: FOE/FRI (Mode 4 interrogation)
        foe_fri = static_cast<FOE_FRI_T>(reader.readBits<2>(bit));

        // Check the FX bit (bit 0) of the second octet for the third octet.
        fx = reader.readBit(bit);

        consumed++; // Now we've finished 2 bytes

        if (fx) {
            if (consumed >= data.size()) return 0;

            // --- Third Octet (EP_VAL fields) ---
            // ADSB: Bit 8 (EP) and Bit 7 (Value)
            adsb.ep  = reader.readBit(bit);
            adsb.val = static_cast<uint8_t>(reader.readBit(bit));

            // SCN: Bit 6 (EP) and Bit 5 (Value)
            scn.ep   = reader.readBit(bit);
            scn.val  = static_cast<uint8_t>(reader.readBit(bit));

            // PAI: Bit 4 (EP) and Bit 3 (Value)
            pai.ep   = reader.readBit(bit);
            pai.val  = static_cast<uint8_t>(reader.readBit(bit));

            reader.skipBits(bit, 1); // Bit 2 (Spare)

            fx = reader.readBit(bit); // Bit 1: Extension
            consumed++;

            if (fx) {
                if (consumed >= data.size()) return 0;

                // --- Fourth Octet (EP_VAL fields) ---
                // ACASXV: 1 bit for EP, 4 bits for VAL (Total 5 bits)
                acasxv.ep  = reader.readBit(bit);
                acasxv.val = static_cast<uint8_t>(reader.readBits<4>(bit));

                // POXPR: 1 bit for EP, 1 bit for VAL (Total 2 bits)
                poxpr.ep  = static_cast<uint8_t>(reader.readBit(bit));
                poxpr.val = static_cast<uint8_t>(reader.readBit(bit));

                fx = reader.readBit(bit);
                consumed++;

                if (fx) {
                    if (consumed >= data.size()) return 0;

                    // --- Fifth Octet ---
                    // Bits 8-7: POACT (EP + VAL)
                    poact.ep  = reader.readBit(bit);         // Bit 8
                    poact.val = static_cast<uint8_t>(reader.readBit(bit));  // Bit 7

                    // Bits 6-5: DTFXPR (EP + VAL)
                    dtfxpr.ep  = reader.readBit(bit);        // Bit 6
                    dtfxpr.val = static_cast<uint8_t>(reader.readBit(bit)); // Bit 5

                    // Bits 4-3: DTFACT (EP + VAL)
                    dtfact.ep  = reader.readBit(bit);        // Bit 4
                    dtfact.val = static_cast<uint8_t>(reader.readBit(bit)); // Bit 3

                    // Bit 2: Spare (set to 0)
                    reader.skipBits(bit, 1);                 // Bit 2

                    // Bit 1: FX Extension bit
                    fx = reader.readBit(bit);                 // Bit 1
                    consumed++;

                    if (fx) {
                        if (consumed >= data.size()) return 0;
                        irmpr.ep   = reader.readBit(bit);    // Bit 8
                        irmpr.val  = static_cast<uint8_t>(reader.readBit(bit)); // Bit 7
                        irmact.ep  = reader.readBit(bit);    // Bit 6
                        irmact.val = static_cast<uint8_t>(reader.readBit(bit)); // Bit 5

                        reader.skipBits(bit, 3);             // Spares
                        fx = reader.readBit(bit);            // Bit 1
                        consumed++;

                        // Extended Chain
                        while (fx) {
                            // Before we move to the next byte, we check if it exists.
                            // 'consumed' currently holds the count of bytes ALREADY read.
                            // If 'consumed' equals 'data.size()', there is no next byte.
                            if (consumed >= data.size()) return 0;

                            // We only care about the FX bit in subsequent octets
                            reader.skipBits(bit, 7);
                            fx = reader.readBit(bit);

                            // We successfully finished another byte
                            consumed++;
                        }
                    }
                }
            }
        }
    }

    AsterixDataItemHandlerBase::decode(data);
    return consumed;
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
 * @param data The raw data buffer for this item (4 bytes).
 */
size_t I048_040_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    range   = readBigEndian<uint16_t>(data.data());
    azimuth = readBigEndian<uint16_t>(data.data() + 2);

    return AsterixDataItemHandlerFixedLength::decode(data);
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
 * @param data The raw data buffer for this item (2 bytes).
 */
size_t I048_070_Handler::decode(std::string_view data) {
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
 * @brief Handler for ASTERIX Data Item I048/090, Flight Level in Binary Representation.
 *
 * Contains the Mode-C code, which represents the flight level (altitude) of the target.
 *
 * The 14-bit value is scaled by 25 ft and converted to meters.
 *
 * @param data The raw data buffer for this item (2 bytes).
 */
size_t I048_090_Handler::decode(std::string_view data) {
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

size_t I048_130_Handler::decode(std::string_view data) {
    size_t indicatorLen = calculateIndicatorLen(data);
    if (indicatorLen == 0 || indicatorLen > data.size()) return 0;

    std::string_view subFieldsData = data.substr(indicatorLen);
    size_t totalSize = indicatorLen;

    uint8_t indicator = static_cast<uint8_t>(data[0]);

    // Bit 8: SRL
    if (indicator & 0x80) {
        // Decode now returns the size. We use it to advance immediately.
        size_t subSize = srl.decode(subFieldsData);

        if (subSize == 0) return 0;

        subFieldsData = subFieldsData.substr(subSize);
        totalSize += subSize;
    }

    // Bit 7: SRR
    if (indicator & 0x40) {
        // Decode now returns the size. We use it to advance immediately.
        size_t subSize = srr.decode(subFieldsData);

        if (subSize == 0) return 0;

        subFieldsData = subFieldsData.substr(subSize);
        totalSize += subSize;
    }

    // Bit 6: SAM
    if (indicator & 0x20) {
        // Decode now returns the size. We use it to advance immediately.
        size_t subSize = sam.decode(subFieldsData);

        if (subSize == 0) return 0;

        subFieldsData = subFieldsData.substr(subSize);
        totalSize += subSize;
    }

    // Bit 5: PRL
    if (indicator & 0x10) {
        // Decode now returns the size. We use it to advance immediately.
        size_t subSize = prl.decode(subFieldsData);

        if (subSize == 0) return 0;

        subFieldsData = subFieldsData.substr(subSize);
        totalSize += subSize;
    }

    // Bit 4: PAM
    if (indicator & 0x08) {
        // Decode now returns the size. We use it to advance immediately.
        size_t subSize = pam.decode(subFieldsData);

        if (subSize == 0) return 0;

        subFieldsData = subFieldsData.substr(subSize);
        totalSize += subSize;
    }

    // Bit 3: RPD
    if (indicator & 0x04) {
        // Decode now returns the size. We use it to advance immediately.
        size_t subSize = rpd.decode(subFieldsData);

        if (subSize == 0) return 0;

        subFieldsData = subFieldsData.substr(subSize);
        totalSize += subSize;
    }

    // Bit 2: APD
    if (indicator & 0x02) {
        // Decode now returns the size. We use it to advance immediately.
        size_t subSize = apd.decode(subFieldsData);

        if (subSize == 0) return 0;

        subFieldsData = subFieldsData.substr(subSize);
        totalSize += subSize;
    }


    for (size_t octetIdx = 1; octetIdx < indicatorLen; ++octetIdx) {
        uint8_t ind = static_cast<uint8_t>(data[octetIdx]);
        if (ind > 1) return 0;
    }

    AsterixDataItemHandlerBase::decode(data);
    return totalSize;
}

/**
 * @brief Handler for ASTERIX Data Item I048/042, Calculated Position in Cartesian Co-ordinates.
 * * This item provides the calculated position in 1/128 NM increments.
 * The values are 16-bit signed integers.
 */
size_t I048_042_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // Read X and Y as signed 16-bit integers (Big Endian)
    // Read as unsigned first to satisfy ntohs, then cast to signed int16_t
    x = static_cast<int16_t>(readBigEndian<uint16_t>(data.data()));
    y = static_cast<int16_t>(readBigEndian<uint16_t>(data.data() + 2));

    return AsterixDataItemHandlerFixedLength::decode(data);
}

size_t I048_030_Handler::decode(std::string_view data) {
    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7;
    size_t consumed = 0;
    bool fx = true;

     while (fx && consumed < data.size()) {
         // Read 7 bits for the code
         uint8_t val = static_cast<uint8_t>(reader.readBits<7>(bit));
         codes.push_back(static_cast<WarningCode>(val));

         // Read the FX bit (Bit 1)
         fx = reader.readBit(bit);
         consumed++;
     }

     AsterixDataItemHandlerBase::decode(data);
     return consumed;
}

/**
 * @brief Handler for I048/080, Mode-3/A Code Confidence Indicator.
 * Extracts the 12-bit confidence mask from the 2-byte field.
 */
size_t I048_080_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // Read the 16-bit block as unsigned
    auto rawValue = readBigEndian<uint16_t>(data.data());

    // Mask out the 4 spare bits (16-13) to isolate the 12 confidence bits
    confidenceMask = rawValue & 0x0FFF;

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for I048/100 Mode-C Gillham Code and Confidence.
 * Octet 1-2: V G 0 0 C1 A1 C2 A2 C4 A4 B1 D1 B2 D2 B4 D4
 * Octet 3-4: 0 0 0 0 QC1 QA1 QC2 QA2 QC4 QA4 QB1 QD1 QB2 QD2 QB4 QD4
 */
size_t I048_100_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // Use uint16_t to read octets 1-2 and 3-4
    uint16_t firstHalf = readBigEndian<uint16_t>(data.data());
    uint16_t secondHalf = readBigEndian<uint16_t>(data.data() + 2);

    // Status bits from the first half
    validated = (firstHalf & 0x8000) == 0; // Bit 32: 0=Validated
    garbled   = (firstHalf & 0x4000) != 0; // Bit 31: 1=Garbled

    // Extract the 12-bit Gillham Code (Bits 28-17)
    grayCode = firstHalf & 0x0FFF;

    // Extract the 12-bit Confidence Indicators (Bits 12-1)
    // 0 = High confidence, 1 = Low confidence
    confidence = secondHalf & 0x0FFF;

    return AsterixDataItemHandlerFixedLength::decode(data);
}

// ----------------------------------------------------------------------------------

/**
 * @brief Handler for ASTERIX Data Item I048/110, Height from a 3D-Radar
 *
 * Contains the height (altitude) of the target is 25ft.
 *
 * @param report The target `Asterix048Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
size_t I048_110_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    auto flightLevelTemp = readBigEndian<uint16_t>(data.data());

    // Clear the reserved bits and extract the 14-bit value.
    flightLevelTemp &= 0x3FFF;

    // The height value is a signed 14-bit integer, so perform sign extension.
    // If the MSB of the 14-bit value (bit 13, mask 0x2000) is set,
    // we set the upper bits (15 and 14) to 1 to complete the 16-bit sign extension.
    if (flightLevelTemp & 0x2000) {
        flightLevelTemp |= 0xC000;
    }

    height = static_cast<int16_t>(flightLevelTemp);

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for ASTERIX Data Item I048/055, Mode-1 Code.
 * According to Edition 1.32:
 * Bit-8 (V): 0 = Validated, 1 = Not Validated
 * Bit-7 (G): 0 = Default, 1 = Garbled
 * Bit-6 (L): 0 = Default, 1 = Local
 * Bits 5-1: Mode-1 Code (Octal digits A and B)
 */
size_t I048_055_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7; // Start at MSB (Bit 8)

    // Decode status bits
    validated = !reader.readBit(bit);
    garbled   = reader.readBit(bit);
    local     = reader.readBit(bit);

    // Extract remaining 5 bits for the code
    code = static_cast<uint8_t>(reader.readBits<5>(bit));

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for ASTERIX Data Item I048/050, Mode-2 Code.
 * Bits 16-14: Status (V, G, L)
 * Bits 12-1: 12-bit octal code
 */
size_t I048_050_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7;

    // Bit-16 (V): 0 = Validated, 1 = Not Validated
    validated = !reader.readBit(bit);
    // Bit-15 (G): 0 = Default, 1 = Garbled code
    garbled   = reader.readBit(bit);
    // Bit-14 (L): 0 = Default, 1 = Local network code
    local     = reader.readBit(bit);

    // Read the 16-bit block as unsigned to keep the compiler happy
    auto rawValue = readBigEndian<uint16_t>(data.data());
    // Mask out the status bits to get the 12-bit octal code
    code = rawValue & 0x0FFF;

    return AsterixDataItemHandlerFixedLength::decode(data);
}

size_t I048_065_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // Bits 8-6 are Spare. Bits 5-1 are the confidence indicators.
    confidenceMask = static_cast<uint8_t>(data[0]) & 0x1F;

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for ASTERIX Data Item I048/060, Mode-2 Code Confidence Indicator.
 * Bits 16-13: Spare
 * Bits 12-1: Confidence indicators for each bit of the Mode-2 code.
 */
size_t I048_060_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // Read the 16-bit block as unsigned to avoid warnings.
    auto rawValue = readBigEndian<uint16_t>(data.data());

    // Mask out the spare bits (16, 15, 14, 13) to isolate the 12-bit array.
    confidenceMask = rawValue & 0x0FFF;

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
