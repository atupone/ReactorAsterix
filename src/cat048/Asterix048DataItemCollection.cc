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
void I048_020_Handler::decodePrimary(std::string_view data) {
    const uint8_t octet = static_cast<uint8_t>(data[0]);

    // Decode the 3 bits of the TYP (bits 8, 7 and 6).
    typ = static_cast<TYP_T>((octet >> 5) & 0x07);

    // Decode the SIM bit (Simulation - bits 5).
    sim = (octet >> 4) & 0x01;

    // Decode the RDP bit (Radar Display Processor Chain - bits 4).
    rdp = (octet >> 3) & 0x01;

    // Decode the SPI bit (Special Position Identification - bit 3).
    spi = (octet >> 2) & 0x01;

    // Decode the RAB bit (Report from Aircraft Transponder - bit 2).
    rab = (octet >> 1) & 0x01;
}

void I048_020_Handler::decodeExtension(uint32_t index, std::string_view data) {
    const uint8_t octet = static_cast<uint8_t>(data[0]);
    if (index == 1) {
        // Decode TST bit (Test, bit 8 2nd byte)
        tst = (octet >> 7) & 0x01;

        // decode ERR bit (Extended Range, bits 7).
        err = (octet >> 6) & 0x01;

        // decode XPP bit (X-Pulse - bits 6).
        xpp = (octet >> 5) & 0x01;

        me  = (octet >> 4) & 0x01;

        mi  = (octet >> 3) & 0x01; // Bit 4: Military Identification

        // Bits 3-2: FOE/FRI (Mode 4 interrogation)
        foe_fri = static_cast<FOE_FRI_T>((octet >> 1) & 0x03);

    } else if (index == 2) {
        // --- Third Octet (EP_VAL fields) ---
        // ADSB: Bit 8 (EP) and Bit 7 (Value)
        adsb.ep  = (octet >> 7) & 0x01;
        adsb.val = (octet >> 6) & 0x01;

        // SCN: Bit 6 (EP) and Bit 5 (Value)
        scn.ep   = (octet >> 5) & 0x01;
        scn.val  = (octet >> 4) & 0x01;

        // PAI: Bit 4 (EP) and Bit 3 (Value)
        pai.ep   = (octet >> 3) & 0x01;
        pai.val  = (octet >> 2) & 0x01;

    } else if (index == 3) {
        // --- Fourth Octet (EP_VAL fields) ---
        // ACASXV: 1 bit for EP, 4 bits for VAL (Total 5 bits)
        acasxv.ep  = (octet >> 7) & 0x01;
        acasxv.val = (octet >> 3) & 0x0F;

        // POXPR: 1 bit for EP, 1 bit for VAL (Total 2 bits)
        poxpr.ep   = (octet >> 2) & 0x01;
        poxpr.val  = (octet >> 1) & 0x01;

    } else if (index == 4) {
        // --- Fifth Octet ---
        // Bits 8-7: POACT (EP + VAL)
        poact.ep   = (octet >> 7) & 0x01;
        poact.val  = (octet >> 6) & 0x01;

        // Bits 6-5: DTFXPR (EP + VAL)
        dtfxpr.ep  = (octet >> 5) & 0x01;
        dtfxpr.val = (octet >> 4) & 0x01;

        // Bits 4-3: DTFACT (EP + VAL)
        dtfact.ep  = (octet >> 3) & 0x01;
        dtfact.val = (octet >> 2) & 0x01;

    } else if (index == 5) {
        irmpr.ep   = (octet >> 7) & 0x01;
        irmpr.val  = (octet >> 6) & 0x01;
        irmact.ep  = (octet >> 5) & 0x01;
        irmact.val = (octet >> 4) & 0x01;
    }
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

// Initialize the Compound base with the list of sub-item pointers
I048_130_Handler::I048_130_Handler()
    : AsterixDataItemHandlerCompound({&srl, &srr, &sam, &prl, &pam, &rpd, &apd}) {
        name = "I048/130 Radar Plot Characteristics";
}

/**
 * @brief Decodes Subfield #1: SSR Plot Runlength
 */
size_t I048_130_Handler::SRL::decode(std::string_view data) {
    if (data.size() < static_cast<size_t>(fixedSize)) return 0;
    ssrRunlength = static_cast<uint8_t>(data[0]);
    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Decodes Subfield #2: Number of Received Replies for (M)SSR
 */
size_t I048_130_Handler::SRR::decode(std::string_view data) {
    if (data.size() < static_cast<size_t>(fixedSize)) return 0;
    numSsrReplies = static_cast<uint8_t>(data[0]);
    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Decodes Subfield #3: Amplitude of (M)SSR Reply
 */
size_t I048_130_Handler::SAM::decode(std::string_view data) {
    if (data.size() < static_cast<size_t>(fixedSize)) return 0;
    ssrReplyAmplitude = static_cast<int8_t>(data[0]);
    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Decodes Subfield #4: Primary Plot Runlength
 */
size_t I048_130_Handler::PRL::decode(std::string_view data) {
    if (data.size() < static_cast<size_t>(fixedSize)) return 0;
    psrRunlength = static_cast<uint8_t>(data[0]);
    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Decodes Subfield #5: Amplitude of Primary Plot
 */
size_t I048_130_Handler::PAM::decode(std::string_view data) {
    if (data.size() < static_cast<size_t>(fixedSize)) return 0;
    psrReplyAmplitude = static_cast<int8_t>(data[0]);
    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Decodes Subfield #6: Difference in Range between PSR and SSR
 */
size_t I048_130_Handler::RPD::decode(std::string_view data) {
    if (data.size() < static_cast<size_t>(fixedSize)) return 0;
    rangeDifference = static_cast<int8_t>(data[0]);
    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Decodes Subfield #7: Difference in Azimuth between PSR and SSR
 */
size_t I048_130_Handler::APD::decode(std::string_view data) {
    if (data.size() < static_cast<size_t>(fixedSize)) return 0;
    azimuthDifference = static_cast<int8_t>(data[0]);
    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for I048/220, Aircraft Address.
 * 24-bit Mode S address, typically represented as a hex value.
 */
size_t I048_220_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    auto* udata = reinterpret_cast<const uint8_t*>(data.data());
    // Combine 3 bytes into a single integer
    address = (static_cast<uint32_t>(udata[0]) << 16) |
        (static_cast<uint32_t>(udata[1]) << 8)  |
        (static_cast<uint32_t>(udata[2]));

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for I048/240, Aircraft Identification.
 * 8 characters (6 bits each) encoded in 6 bytes.
 */
size_t I048_240_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(data.data());
    FastBitReader reader(raw);
    int bit = 7;

    identification.clear();
    for (int i = 0; i < 8; ++i) {
        uint8_t val = static_cast<uint8_t>(reader.readBits<6>(bit));
        // Map 6-bit value to IA-5/ASCII according to Eurocontrol Spec
        if (val >= 1 && val <= 26) identification += static_cast<char>('A' + val - 1);
        else if (val >= 48 && val <= 57) identification += static_cast<char>('0' + val - 48);
        else if (val == 32) identification += ' ';
        else identification += '?'; // Unknown/Spare
    }

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for I048/161, Track Number
 */
size_t I048_161_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // 12 bits are used for the track number, 4 bits are spare (top bits)
    trackNumber = readBigEndian<uint16_t>(data.data()) & 0x0FFF;

    return AsterixDataItemHandlerFixedLength::decode(data);
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

/**
 * @brief Handler for I048/200, Calculated Track Velocity in Polar Co-ordinates.
 */
size_t I048_200_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    groundSpeed = readBigEndian<uint16_t>(data.data());
    trackAngle  = readBigEndian<uint16_t>(data.data() + 2);

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for I048/170, Track Status.
 * Extended length item.
 */
void I048_170_Handler::decodePrimary(std::string_view data) {
    const uint8_t octet = static_cast<uint8_t>(data[0]);

    cnf = (octet >> 7) & 0x01;
    rad = static_cast<RAD_T>((octet >> 5) & 0x03);
    dou = (octet >> 4) & 0x01;
    mah = (octet >> 3) & 0x01;
    cdm = static_cast<CDM_T>((octet >> 1) & 0x03);
}

void I048_170_Handler::decodeExtension(uint32_t index, std::string_view data) {
    if (index == 1) { // Process only the first extension
        const uint8_t octet = static_cast<uint8_t>(data[0]);

        tre = (octet >> 7) & 0x01;
        gho = (octet >> 6) & 0x01;
        sup = (octet >> 5) & 0x01;
        tcc = (octet >> 4) & 0x01;
    }
}

size_t I048_210_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    const uint8_t* octets = reinterpret_cast<const uint8_t*>(data.data());

    sigmaX = octets[0];
    sigmaY = octets[1];
    sigmaV = octets[2];
    sigmaH = octets[3];

    return AsterixDataItemHandlerFixedLength::decode(data);
}

void I048_030_Handler::decodePrimary(std::string_view data) {
    // ASTERIX bit 8-2 is the error code.
    // In C++ (0-7 indexing), this is index 7 down to 1.
    // Shift by 1 to move bit 2 to the 0th position and mask 7 bits (0x7F).
    uint8_t code = (static_cast<uint8_t>(data[0]) >> 1) & 0x7F;
    warningCodes.push_back(static_cast<WarningCode>(code));
}

void I048_030_Handler::decodeExtension(uint32_t /*index*/, std::string_view data) {
    // Every extension byte in I048/030 is just another 7-bit error code.
    uint8_t code = (static_cast<uint8_t>(data[0]) >> 1) & 0x7F;
    warningCodes.push_back(static_cast<WarningCode>(code));
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

size_t I048_120_Handler::CAL::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // Read the 16-bit block
    uint16_t rawValue = readBigEndian<uint16_t>(data.data());

    // Bit-16: Validity (D)
    isDoubtful = (rawValue >> 15) & 0x01;

    // Bits 10-1: 10-bit Two's Complement Speed
    // Extract the lower 10 bits
    uint16_t extracted = rawValue & 0x03FF;

    // Manual Sign Extension for 10-bit to 16-bit
    // If bit 10 (0x0200) is set, it's a negative number
    if (extracted & 0x0200) {
        speed = static_cast<int16_t>(extracted | 0xFC00); // Fill bits 11-16 with 1s
    } else {
        speed = static_cast<int16_t>(extracted);
    }

    return AsterixDataItemHandlerFixedLength::decode(data);
}

size_t I048_230_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    const uint8_t oct1 = static_cast<uint8_t>(data[0]);
    const uint8_t oct2 = static_cast<uint8_t>(data[1]);

    // Octet 1:
    // Bits 8-6: COM (Shift 5, Mask 0x07)
    // Bits 5-3: STAT (Shift 2, Mask 0x07)
    // Bit 2: SI (Shift 1, Mask 0x01)
    com  = static_cast<CommsCapability>((oct1 >> 5) & 0x07);
    stat = static_cast<FlightStatus>((oct1 >> 2) & 0x07);
    si   = (oct1 >> 1) & 0x01;

    // Octet 2:
    // Bit 8: MSSCC, Bit 7: ARC, Bit 6: AIC, Bit 5: B1A, Bits 4-1: B1B
    msscc = (oct2 >> 7) & 0x01;
    arc   = (oct2 >> 6) & 0x01;
    aic   = (oct2 >> 5) & 0x01;
    b1a   = (oct2 >> 4) & 0x01;
    b1b   = oct2 & 0x0F;

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Handler for I048/260, ACAS Resolution Advisory Report.
 */
size_t I048_260_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    const uint8_t* udata = reinterpret_cast<const uint8_t*>(data.data());

    // Pack 7 bytes into a single 64-bit integer
    acasMsg = 0;
    for (size_t i = 0; i < 7; ++i) {
        acasMsg = (acasMsg << 8) | udata[i];
    }

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
