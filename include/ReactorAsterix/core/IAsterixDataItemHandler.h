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

// System headers
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ReactorAsterix {

    struct AsterixStats; // Forward declaration for diagnostic support

/**
 * @class IAsterixDataItemHandler
 * @brief An interface for classes that handle the decoding of a single ASTERIX
 * data item.
 *
 * This abstract base class defines the contract for decoding a specific data
 * item. Concrete classes that implement this interface are responsible for
 * extracting and interpreting the data for a single item from the ASTERIX
 * data stream.
 *
 * @tparam T The type of the context object (e.g., Asterix1Report) that will
 * receive the decoded information.
 */
template <typename T>
class IAsterixDataItemHandler {
    public:
        /**
         * @brief Virtual destructor to ensure proper cleanup of derived classes.
         */
        virtual ~IAsterixDataItemHandler() = default;

        /**
         * @brief Links the central statistics to this handler.
         * * This allows individual item decoders to report errors or
         * unimplemented features to the global stats object.
         */
        virtual void setStats(AsterixStats& s) = 0; // Added for diagnostic tracking

        /**
         * @brief Decodes the ASTERIX data item.
         *
         * This method should be implemented by concrete classes to parse a single
         * data item and write the result to a context object. It assumes the
         * correct amount of data is available based on a prior call to `getSize()`.
         *
         * @param context A reference to the context object (e.g., an `Asterix1Report`)
         * where the decoded information will be stored.
         * @param data A pointer to the start of the data item.
         */
        virtual void decode(T& context, std::string_view data) const = 0;

        /**
         * @brief Returns the size of the data item in bytes.
         *
         * This method is used to determine how many bytes to read from the data stream.
         * For variable-length items, the implementation will need to inspect the
         * data content (e.g., using the FX bit) to calculate the size.
         *
         * @param data A pointer to the start of the data item.
         * @param dataLeft The remaining size of the data stream.
         * @return The size of the data item in bytes.
         */
        [[nodiscard]] virtual size_t getSize(std::string_view data) const = 0;

        /**
         * @brief Checks if the data item is mandatory.
         *
         * A mandatory item must be present in the data record; otherwise, an
         * exception should be thrown.
         *
         * @return `true` if the item is mandatory, `false` otherwise.
         */
        virtual bool isMandatory() const = 0;

        /**
         * @brief Gets the human-readable name of the data item.
         *
         * @return The name of the data item as a string.
         */
        virtual std::string_view getName() const = 0;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
