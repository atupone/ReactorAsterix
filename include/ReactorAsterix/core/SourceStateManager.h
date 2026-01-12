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
#include <map>
#include <cstdint>
#include <optional>

// Library headers
#include "ReactorAsterix/core/SourceIdentifier.h"

namespace ReactorAsterix {

class SourceStateManager {
    public:
        /**
         * @brief Returns the last known 32-bit TOD or 0xFFFFFFFF if unknown.
         */
        [[nodiscard]] std::optional<uint32_t> getReferenceTime(const SourceIdentifier& si) const {
            if (const auto it = sources.find(si); it != sources.end()) {
                return it->second;
            }
            return std::nullopt;
        }

        /**
         * @brief Updates the stored 32-bit TOD for a specific source.
         * Can be called by CAT 002, 048, 062, etc., whenever a full TOD is available.
         */
        void updateSourceTime(const SourceIdentifier& si, uint32_t fullTod) {
            sources.insert_or_assign(si, fullTod);
        }

    private:
        std::map<SourceIdentifier, uint32_t> sources;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
