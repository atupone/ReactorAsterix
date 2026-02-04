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
#include <ReactorAsterix/core/AsterixMessage.h>

// System headers
#include <cstdint>

namespace ReactorAsterix {

/**
 * @class Asterix048Report
 * @brief Container for decoded Category 048 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix048Report final : public AsterixMessage {
    public:
        Asterix048Report() = default;
        ~Asterix048Report() override = default;

        enum Presence : uint16_t {
            HAS_MODE_3A      = 1 << 0,
            HAS_SR_HEIGHT    = 1 << 1,
            HAS_SSR_HEIGHT   = 1 << 2,
            HAS_MEASUR_COORD = 1 << 3,
        };

        uint16_t presenceMask = 0;

// --- Target Report Descriptor bits
        // Enumeration for TYP
        enum class TYP_T : uint8_t {
            NO_DETECTION = 0,
            SINGLE_PSR_DETECTION   = 1,
            SINGLE_SSR_DETECTION   = 2,
            SSR_PSR_DETECTION      = 3,
            SINGLE_MODES_ALL_CALL  = 4,
            SINGLE_MODES_ROLL_CALL = 5,
            MODES_ALL_CALL_PSR     = 6,
            MODES_ROLL_CALL_PSR    = 7
        };


// ---  Measured Position in Polar Coordinates
        // Physical Data (converted from raw bits)
        double range{0.0};   // Meters
        double azimuth{0.0}; // Radians

        // SSR / Mode Data
        struct {
            uint16_t code;
            bool validated : 1;
            bool garbled   : 1;
            bool local     : 1;;
        } mode3A;

// ---  Flight Level in Binary Representation
        struct {
            double height; // meters
            bool validated : 1;
            bool garbled   : 1;
        } ssrHeight;

        double srHeight{0.0};

        // Check: if (report.has(Asterix1Report::HAS_MODE_3A)) ...
        bool has(Presence p) const { return presenceMask & p; }


        TYP_T typ{TYP_T::NO_DETECTION};

        bool spi{false};
        bool me{false};

// --- Target Report Descriptor setter
        void setTYP(int _typ) {
            typ = static_cast<TYP_T>(_typ);
        }

        void setSPI(bool _spi) {
            spi = _spi;
        }

// --- Mode-3/A Code in Octal Representation setter
        void setMode3A(uint16_t code, bool v, bool g, bool l) {
            mode3A = {code, v, g, l};
            presenceMask |= HAS_MODE_3A;
        }

// --- Mode-C Code in Binary Representation setter
        void setSSRHeight(int16_t height, bool v, bool g) {
            // The resolution is 25 feet. The scale factor converts
            // the signed 16-bit value (interpreted as a flight level) to meters.
            constexpr double HEIGHT_SCALE = 25.0 * 0.3048;

            ssrHeight = {height * HEIGHT_SCALE, v, g};
            presenceMask |= HAS_SSR_HEIGHT;
        }

// --- 3D Height
        void setSRHeight(int16_t height) {
            // The resolution is 25 feet. The scale factor converts
            // the signed 16-bit value (interpreted as a flight level) to meters.
            constexpr double HEIGHT_SCALE = 25.0 * 0.3048;

            srHeight = height * HEIGHT_SCALE;
            presenceMask |= HAS_SR_HEIGHT;
        }

// --- MeasuredCoordinate
        void setMeasuredCoordinates(uint16_t rawRange, uint16_t rawAzimuth) {
            // Range: LSB = 1/256 NM converted to meters
            // 1852.0 is the standard Nautical Mile to Meters conversion
            range = (static_cast<double>(rawRange) / 256.0) * 1852.0;

            // Azimuth: LSB = (pi/4) / 8192 radians
            // Which is 2*PI / 65536
            constexpr double AZIMUTH_SCALE = 0.00009587379; // (M_PI / 32768.0)
            azimuth = static_cast<double>(rawAzimuth) * AZIMUTH_SCALE;

            presenceMask |= HAS_MEASUR_COORD;
        }

        void setME(bool _me) {
            me = _me;
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
