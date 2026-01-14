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
template <typename T>
class AsterixDataItemHandlerBase : public IAsterixDataItemHandler<T> {
    public:
        AsterixDataItemHandlerBase() : mandatory(false), name("Unknown Item") {};
        virtual ~AsterixDataItemHandlerBase() = default;

        /**
         * @brief Default implementation of decode does nothing.
         * Useful for reserved or ignored items.
         */
        void decode([[maybe_unused]] T& context, [[maybe_unused]] std::string_view data) const override {};

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

        void setStats(AsterixStats& s) override {
            this->stats_ptr = &s;
        }

        // Helper for derived classes to get the reference
        AsterixStats& getStats() const {
            return *stats_ptr;
        }

    protected:
        bool mandatory;

        std::string_view name;

        AsterixStats* stats_ptr = nullptr; // Store the pointer for use in subclasses
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
