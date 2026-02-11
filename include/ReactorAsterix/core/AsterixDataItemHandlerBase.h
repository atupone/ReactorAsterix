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
#include <ReactorAsterix/core/IAsterixDataItemHandler.h>

// SYstem headers
#include <string_view>

namespace ReactorAsterix {

/**
 * @class AsterixDataItemHandlerBase
 * @brief An abstract base class that provides default implementations for common
 * IAsterixDataItemHandler methods.
 *
 * This class is designed to reduce boilerplate for concrete data item handlers.
 * It provides default implementations for `isMandatory()` and `getName()`, which
 * can be overridden by derived classes. The `decode()` method is also provided
 * with a do-nothing implementation for data items that are not meant to be decoded
 * but still need to be handled for size determination (e.g., reserved items).
 */
class AsterixDataItemHandlerBase : public IAsterixDataItemHandler {
    public:
        AsterixDataItemHandlerBase() : name("Unknown Item") {};
        virtual ~AsterixDataItemHandlerBase() = default;

        /**
         * @brief Default implementation of decode does nothing.
         * Useful for reserved or ignored items.
         */
        size_t decode([[maybe_unused]] std::string_view data) override
        {
            presence = true;
            return 0;
        };

        // By default, a data item is NOT mandatory.
        // Derived classes only need to override this if the item is mandatory.
        /**
         * @brief Check if the item is protocol-mandatory.
         */
        bool isMandatory() const override { return mandatory; }

        /**
         * @brief Returns the human-readable name of the item.
         */
        std::string_view getName() const override { return name; };

        static constexpr bool mandatory = false; // Default: items are optional

    protected:
        std::string_view name;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
