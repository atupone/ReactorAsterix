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
#include <memory>
#include <vector>

namespace ReactorAsterix {

/**
 * @class AsterixDataItemHandlerCompound
 * @brief Base class for compound items where sub-item order matches indicator bits.
 */
class AsterixDataItemHandlerCompound : public AsterixDataItemHandlerBase {
    public:
        /**
         * @brief Constructor accepting an ordered list of sub-item handlers.
         * Index 0 = Octet 1, Bit 7 (MSB)
         * Index 6 = Octet 1, Bit 1
         * Index 7 = Octet 2, Bit 7 (MSB), etc.
         */
        explicit AsterixDataItemHandlerCompound(
                std::vector<AsterixDataItemHandlerBase*> subItems)
            : m_subItems(std::move(subItems)) {}
        ~AsterixDataItemHandlerCompound() override = default;

        [[nodiscard]] size_t decode(std::string_view data) override {
            size_t indicatorLen = calculateIndicatorLen(data);
            if (indicatorLen == 0 || indicatorLen > data.size()) return 0;

            std::string_view subFieldsData = data.substr(indicatorLen);
            size_t totalSize = indicatorLen;

            auto* currentHandler = m_subItems.data();
            size_t handlersLeft = m_subItems.size();

            for (size_t octetIdx = 0; octetIdx < indicatorLen; ++octetIdx) {
                uint8_t indicator = static_cast<uint8_t>(data[octetIdx]);

                for (int bit = 7; bit >= 1; --bit) {
                    bool isPresence = (indicator >> bit) & 0x01;

                    if (isPresence) {
                        if (handlersLeft == 0 || !(*currentHandler)) return 0;

                        // Decode now returns the size. We use it to advance immediately.
                        size_t subSize = (*currentHandler)->decode(subFieldsData);

                        if (subSize == 0) return 0;

                        subFieldsData = subFieldsData.substr(subSize);
                        totalSize += subSize;
                    }

                    if (handlersLeft > 0) {
                        handlersLeft--;
                        currentHandler++;
                    }
                }
            }

            AsterixDataItemHandlerBase::decode(data);
            return totalSize;
        }

    protected:
        std::vector<AsterixDataItemHandlerBase*> m_subItems;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
