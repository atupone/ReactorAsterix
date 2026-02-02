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
#include <ReactorAsterix/cat034/Asterix034DataItemCollection.h>

// System headers
#include <cassert>

// Library headers
#include <ReactorAsterix/core/EndianUtils.h>
#include <ReactorAsterix/cat034/Asterix034Report.h>

namespace ReactorAsterix {

/**
 * @brief Handler for ASTERIX Data Item I034/010, Data Source Identifier.
 *
 * This item provides the System Area Code (SAC) and System Identification Code (SIC)
 * to uniquely identify the data source (e.g., the radar station).
 *
 * The first byte is the SAC, and the second byte is the SIC.
 *
 * @param report The target `Asterix034Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I034_010_Handler::decode(Asterix034Report& report, std::string_view data) const {
    if (data.size() < fixedSize) [[unlikely]] {
        return;
    }

    uint8_t sac = static_cast<uint8_t>(data[0]);
    uint8_t sic = static_cast<uint8_t>(data[1]);
    report.setSourceIdentifier(sac, sic);
}

/**
 * @brief Decodes the 1-byte Message Type (I034/000).
 * 1 = North Marker, 2 = Sector Message
 */
void I034_000_Handler::decode(Asterix034Report& context, std::string_view data) const {
    if (data.size() < fixedSize) [[unlikely]] {
        return;
    }

    context.setMessageType(static_cast<uint8_t>(data[0]));
}

/**
 * @brief Decodes the 3-byte Time of Day (TOD).
 * The TOD value is constructed from the three bytes, where the unit is in
 * 1/128 seconds.
 *
 * @param context The target context object (Asterix034Report) to store the result.
 * @param data The raw data buffer containing the 3 bytes of TOD.
 */
void I034_030_Handler::decode(Asterix034Report& context, std::string_view data) const {
    if (data.size() < fixedSize) [[unlikely]] {
        return;
    }

    context.TOD = decodeBigEndian<uint32_t>(data.substr(0, 3));
}

void I034_020_Handler::decode(Asterix034Report& context, std::string_view data) const {
    if (data.size() < fixedSize) [[unlikely]] {
        return;
    }

    context.sectorNumber = static_cast<uint8_t>(data[0]);
}

void I034_041_Handler::decode(Asterix034Report& context, std::string_view data) const {
    if (data.size() < fixedSize) [[unlikely]] return;

    // LSB = 1/128 second
    uint16_t periodRaw = decodeBigEndian<uint16_t>(data);
    if (periodRaw > 0) {
        // Convert period to RPM: (60 sec) / (periodRaw / 128.0)
        context.setAntennaSpeed(7680.0f / static_cast<float>(periodRaw));
    }
}

void I034_120_Handler::decode(Asterix034Report& context, std::string_view data) const {
    if (data.size() < fixedSize) [[unlikely]] {
        return;
    }

    // Height: 16-bit unsigned, LSB = 1 meter
    context.height = (static_cast<uint8_t>(data[0]) << 8) | static_cast<uint8_t>(data[1]);

    // Latitude (Bytes 3-5): 3-byte signed, LSB = 180/2^23 degrees
    constexpr double scaling = 180.0 / 8388608.0; 
    context.latitude = decode24BitSigned(data.substr(2, 3)) * scaling;

    // Longitude (Bytes 6-8): 3-byte signed
    context.longitude = decode24BitSigned(data.substr(5, 3)) * scaling;
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
