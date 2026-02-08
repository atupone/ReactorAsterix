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
#include <ctime>
#include <string_view>

namespace ReactorAsterix {

    struct AsterixStatsData; // Forward declaration

    /**
     * @class IAsterixCategoryHandler
     * @brief An interface for classes that handle specific ASTERIX categories.
     *
     * This abstract base class defines the contract for processing a single data
     * record belonging to a particular ASTERIX category. Derived classes are
     * responsible for implementing the logic to parse the data item based on its
     * Field Specification (F-spec) and then dispatch the decoded information for
     * further processing (e.g., to a tracking system).
     */
    class IAsterixCategoryHandler {
        public:
            /**
             * @brief Virtual destructor to ensure proper cleanup of derived classes.
             */
            virtual ~IAsterixCategoryHandler() = default;

            /**
             * @brief Handles the processing of a single ASTERIX data record.
             *
             * This method should be implemented by concrete classes to parse a single
             * data record, which includes its F-spec and the subsequent data item.
             * It should encapsulate the specific parsing logic for the data item.
             *
             * @param fspec A pointer to the start of the Field Specification.
             * @param fspecSize The size of the F-spec in bytes.
             * @param data A pointer to the start of the data item.
             * @param dataLeft The remaining size of the data item in bytes.
             * @return size_t The number of bytes consumed by the handler.
             */
            [[nodiscard]]virtual size_t processDataRecord(
                    std::string_view fspec,
                    std::string_view payload,
                    struct timespec ts,
                    AsterixStatsData& localStats) = 0;
    };

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
