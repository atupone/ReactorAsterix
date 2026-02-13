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
#include <string_view>
#include <vector>

// Library headers
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/core/IAsterixCategoryHandler.h>

namespace ReactorAsterix {

/**
 * @class AsterixReport
 * @brief Container for decoded Category data.
 * The client is responsible for converting these values into physical coordinates.
 */
class AsterixReport : public AsterixMessage {
    public:
        /**
         * Decodes Cat fields using the recursive template schema.
         */
        virtual bool process_all_octets(
            std::string_view fspec,
            std::string_view& data,
            AsterixStatsData& stats,
            IAsterixCategoryHandler& parent) = 0;

    protected:
        /**
         * @brief Check if the received FSPEC is long enough.
         * @param fspec The FSPEC from the data stream.
         * @param min_required The pre-calculated minimum length for this category.
         */
        bool is_fspec_complete(std::string_view fspec, size_t min_required) const {
            return fspec.size() >= min_required;
        }

        static size_t min_fspec_len;
        static bool initialized;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
