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

// Inherits from the main interface
#include <ReactorAsterix/core/AsterixDataItemHandlerBase.h>

// System headers
#include <cstdint>

namespace ReactorAsterix {

/**
 * @class AsterixDataItemHandlerSP
 * @brief Helper template for ASTERIX Special Purpose and Reserved Epansion
 */
class AsterixDataItemHandlerSP : public AsterixDataItemHandlerBase {
    public:
        /**
         * @brief Constructor
         * @param k Size of the repetitive item.
         */
        explicit AsterixDataItemHandlerSP() {}
        ~AsterixDataItemHandlerSP() override = default;

        /**
         * @brief Calculates the total size by reading the First byte
         * Matches the signature in IAsterixDataItemHandler.h.
         */
        inline size_t getSize(std::string_view data) const final {
            if (data.size() < 1) [[unlikely]] {
                return 0;
            }

            size_t totalSize = static_cast<size_t>(data[0]);

            if (data.size() < totalSize) [[unlikely]] {
                return 0;
            }

            return totalSize;
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
