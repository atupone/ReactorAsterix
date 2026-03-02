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
#include <vector>

namespace ReactorAsterix {

/**
 * @class AsterixDataItemHandlerExtendedLength
 * @brief Finalized template for ASTERIX repetitive items.
 * @tparam SubHandler The class responsible for decoding a single instance.
 */
template <typename SubHandler>
class AsterixDataItemHandlerRepetitive : public AsterixDataItemHandlerBase {
    public:
        AsterixDataItemHandlerRepetitive() = default;
        ~AsterixDataItemHandlerRepetitive() override = default;

        size_t decode(std::string_view data) final {
            AsterixDataItemHandlerBase::decode(data);

            if (data.empty()) return 0;

            const uint8_t rep = static_cast<uint8_t>(data[0]);
            if (rep == 0) return 0;

            // Get the constant from the sub-handler type
            constexpr size_t k = SubHandler::FixedLength;

            const size_t totalSize = 1 + static_cast<size_t>(rep) * k;

            if (data.size() < totalSize) return 0;

            entries.clear();
            entries.reserve(rep);

            for (uint8_t i = 0; i < rep; ++i) {
                SubHandler entry;
                // Provide a view of just the k-sized block for this repetition
                std::string_view subData = data.substr(1 + i * k, k);
                entry.decode(subData);
                entries.push_back(std::move(entry));
            }

            return totalSize;
        }

        std::vector<SubHandler>& getEntries() const { return entries; };

    protected:
        std::vector<SubHandler> entries;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
