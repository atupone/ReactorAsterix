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
 * @param data The raw data buffer for this item (2 bytes).
 */
size_t I034_010_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    sac = static_cast<uint8_t>(data[0]);
    sic = static_cast<uint8_t>(data[1]);

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Decodes the 1-byte Message Type (I034/000).
 * 1 = North Marker, 2 = Sector Message
 */
size_t I034_000_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    messageType = static_cast<MESSAGE_TYPE_T>(data[0]);

    return AsterixDataItemHandlerFixedLength::decode(data);
}

/**
 * @brief Decodes the 3-byte Time of Day (TOD).
 * The TOD value is constructed from the three bytes, where the unit is in
 * 1/128 seconds.
 *
 * @param data The raw data buffer containing the 3 bytes of TOD.
 */
size_t I034_030_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // Use a pointer to unsigned to avoid messy casting
    auto* udata = reinterpret_cast<const uint8_t*>(data.data());

    TOD =
        (static_cast<uint32_t>(udata[0]) << 16) |
        (static_cast<uint32_t>(udata[1]) << 8)  |
        (static_cast<uint32_t>(udata[2]));

    return AsterixDataItemHandlerFixedLength::decode(data);
}

size_t I034_020_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    sectorNumber = static_cast<uint8_t>(data[0]);

    return AsterixDataItemHandlerFixedLength::decode(data);
}

size_t I034_041_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // LSB = 1/128 second
    speed = decodeBigEndian<uint16_t>(data);

    return AsterixDataItemHandlerFixedLength::decode(data);
}

size_t I034_120_Handler::decode(std::string_view data) {
    if (data.size() < fixedSize) return 0;

    // Height: 16-bit unsigned, LSB = 1 meter
    height = (static_cast<uint8_t>(data[0]) << 8) | static_cast<uint8_t>(data[1]);

    // Latitude (Bytes 3-5): 3-byte signed, LSB = 180/2^23 degrees
    latitude = decode24BitSigned(data.substr(2, 3));

    // Longitude (Bytes 6-8): 3-byte signed
    longitude = decode24BitSigned(data.substr(5, 3));

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
