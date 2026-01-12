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
#include "ReactorAsterix/cat002/Asterix2DataItemCollection.h"

// System headers
#include <arpa/inet.h>
#include <cmath>
#include <cstring>

// Library headers
#include "ReactorAsterix/cat002/Asterix2Report.h"
#include "ReactorAsterix/core/asterixExceptions.h"

namespace ReactorAsterix {

/**
 * @brief Handler for ASTERIX Data Item I002/010, Data Source Identifier.
 *
 * This item provides the System Area Code (SAC) and System Identification Code (SIC)
 * to uniquely identify the data source (e.g., the radar station).
 *
 * The first byte is the SAC, and the second byte is the SIC.
 *
 * @param report The target `Asterix2Report` object.
 * @param data The raw data buffer for this item (2 bytes).
 */
void I002_010_Handler::decode(Asterix2Report& report, std::string_view data) const {
    // data[0] is the System Area Code (SAC), and data[1] is the System Identification Code (SIC).
    // Explicit cast to avoid sign-conversion warnings
    uint8_t sac = static_cast<uint8_t>(data[0]);
    uint8_t sic = static_cast<uint8_t>(data[1]);
    report.setSourceIdentifier(sac, sic);
}

// ----------------------------------------------------------------------------------

/**
 * @brief Decodes the 3-byte Time of Day (TOD).
 * The TOD value is constructed from the three bytes, where the unit is in
 * 1/128 seconds.
 *
 * @param context The target context object (Asterix2Report) to store the result.
 * @param data The raw data buffer containing the 3 bytes of TOD.
 */
void I002_030_Handler::decode(Asterix2Report& context, std::string_view data) const {
    uint32_t tod = (static_cast<uint32_t>(static_cast<uint8_t>(data[0])) << 16) |
                   (static_cast<uint32_t>(static_cast<uint8_t>(data[1])) << 8)  |
                   (static_cast<uint32_t>(static_cast<uint8_t>(data[2])));

    context.TOD = tod;
}

/**
 * @brief Decodes the 2-byte Antenna Rotation Speed.
 *
 * The raw 2-byte value is converted from network to host byte order,
 * and then divided by 128 to get the speed in RPM.
 *
 * @param context The target context object (Asterix2Report) to store the result.
 * @param data The raw data buffer containing the 2-byte speed value.
 */
void I002_041_Handler::decode(Asterix2Report& context, std::string_view data) const {
    uint16_t speedTemp;
    memcpy(&speedTemp, data.data(), 2);
    // Convert from Network Byte Order (Big-Endian) to Host Byte Order.
    speedTemp = ntohs(speedTemp);

    // The value is in units of 1/128 RPM.
    context.setAntennaSpeed(speedTemp / 128.0f);
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
