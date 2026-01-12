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
#include "ReactorAsterix/core/AsterixDataItemHandlerBase.h"

namespace ReactorAsterix {

/**
 * @class AsterixDataItemHandlerFixedLength
 * @brief Helper for items with a constant, pre-defined byte length.
 */
template <typename T>
class AsterixDataItemHandlerFixedLength : public AsterixDataItemHandlerBase<T> {
    public:
        /**
         * @brief Constructor.
         * @param length The fixed size in bytes.
         */
        explicit AsterixDataItemHandlerFixedLength(uint8_t length) : fixedSize(length) {}

        ~AsterixDataItemHandlerFixedLength() override = default;

        /**
         * @brief Returns the constant size, ignoring the buffer content.
         */
        size_t getSize(std::string_view /*data*/) const final {
            return fixedSize;
        };

    protected:
        uint8_t fixedSize;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
