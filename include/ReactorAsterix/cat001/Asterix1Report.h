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
#include <cmath>

namespace ReactorAsterix {

/**
 * @class Asterix1Report
 * @brief Container for decoded Category 001 data.
 * The client is responsible for converting these values into physical coordinates.
 */
class Asterix1Report final : public AsterixMessage {
    public:
        Asterix1Report() = default;
        ~Asterix1Report() override = default;

        enum Presence : uint16_t {
            HAS_MODE_3A   = 1 << 0,
            HAS_HEIGHT    = 1 << 1,
            HAS_LSP_CLOCK = 1 << 2
        };

        uint16_t presenceMask = 0;

// --- Target Report Descriptor bits
        // Enumeration for SSR/PSR
        enum class SSRPSR_T : uint8_t {
            NO_DETECTION = 0,
            SOLE_PRIMARY_DETECTION = 1,
            SOLE_SECONDARY_DETECTION = 2,
            COMBINED_PRIMARY_AND_SECONDARY_DETECTION = 3
        };

        enum class DS1DS2_T : uint8_t {
            DEFAULT = 0,
            UNLAWFUL_INTERFERENCE = 1,
            RADIO_COMMUNICATION_FAILURE = 2,
            EMERGENCY = 3
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

// ---  Mode-C Code in Binary Representation
        struct {
            double height; // meters
            bool validated : 1;
            bool garbled   : 1;
        } ssrHeight;

        // Check: if (report.has(Asterix1Report::HAS_MODE_3A)) ...
        bool has(Presence p) const { return presenceMask & p; }

        uint16_t todLSP{0};   // I001/141: Truncated Time (LSB = 1/128 s)

        SSRPSR_T ssrpsr{SSRPSR_T::NO_DETECTION};
        DS1DS2_T ds1ds2{DS1DS2_T::DEFAULT};

        bool spi{false};

// --- Target Report Descriptor setter
        void setSSR_PSR(int _ssrpsr) {
            ssrpsr = static_cast<SSRPSR_T>(_ssrpsr);
        }

        void setSPI(bool _spi) {
            spi = _spi;
        }

        void setDs1Ds2(int _ds1ds2) {
            ds1ds2 = static_cast<DS1DS2_T>(_ds1ds2);
        }

// --- Mode-3/A Code in Octal Representation setter
        void setMode3A(uint16_t code, bool v, bool g, bool l) {
            mode3A = {code, v, g, l};
            presenceMask |= HAS_MODE_3A;
        }

// --- Mode-C Code in Binary Representation setter
        void setSSRHeight(double height, bool v, bool g) {
            ssrHeight = {height, v, g};
            presenceMask |= HAS_HEIGHT;
        }

        void setTruncatedTimeOfDay(uint16_t tod) {
            todLSP = tod;
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
