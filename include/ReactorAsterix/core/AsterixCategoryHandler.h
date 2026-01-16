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
#include <ReactorAsterix/core/IAsterixCategoryHandler.h>

// System headers
#include <algorithm>
#include <array>
#include <memory>
#include <vector>

// Libray headers
#include <ReactorAsterix/core/IAsterixDataItemHandler.h>
#include <ReactorAsterix/core/AsterixDiagnostics.h>

namespace ReactorAsterix {

/**
 * @class AsterixCategoryHandler
 * @brief Base template for specific category handlers (e.g., Cat 001).
 * @tparam T The Record type (context) this handler populates.
 */
template <typename T>
class AsterixCategoryHandler : public IAsterixCategoryHandler {
    public:
        /**
         * @brief Virtual destructor to ensure proper cleanup of derived classes
         * and managed handlers.
         */
        virtual ~AsterixCategoryHandler() = default;

        /**
         * @brief Links the central statistics to this handler.
         */
        void setStats(AsterixStats& s) override;

    protected:
        // Pre-computed F-spec where bits are 1 if the item is mandatory
        std::array<uint8_t, 20> mandatoryFspec{};
        size_t mandatoryFspecSize = 0; // Tracks the highest byte index used

        /**
         * @brief Registers the specific data item handlers for the ASTERIX category.
         *
         * This pure virtual method must be implemented by concrete derived classes
         * (e.g., Asterix1Handler, Asterix2Handler) to populate the `handlers` vector
         * with their specific data item handlers.
         */
        virtual void registerHandlers() = 0;

        /**
         * @brief Internal helper for derived classes to register a data item handler.
         * Separates ownership (pool) from fast lookup (array).
         */
        void addHandler(std::unique_ptr<IAsterixDataItemHandler<T>> h, uint8_t frn) {
            if (!h || frn == 0 || frn > MAX_FRNS) return;

            // Link stats if available
            if (this->stats_ptr) {
                h->setStats(*this->stats_ptr);
            }

            if (h->isMandatory()) {
                // Determine which byte and bit in the F-spec this FRN corresponds to
                const size_t uFrn = static_cast<size_t>(frn);
                size_t byteIdx = (uFrn - 1) / 7;
                size_t bitIdx  = 7 - ((uFrn - 1) % 7); // Bits 7 to 1

                // Set the bit in the mandatory mask corresponding to this FRN
                mandatoryFspec[byteIdx] |= static_cast<uint8_t>(1 << bitIdx);
                mandatoryFspecSize = std::max(mandatoryFspecSize, byteIdx + 1);
            }

            // RESET LOGIC: Check if an FRN is already occupied
            // Index is frn - 1 because FRNs start at 1
            if (auto* old = itemLookup[frn - 1]; old != nullptr) {
                // Remove the old owner from the pool
                auto it = std::remove_if(handlerOwnership.begin(), handlerOwnership.end(),
                    [old](const auto& ptr) {return ptr.get() == old; });
                handlerOwnership.erase(it, handlerOwnership.end());
            }

            // Update the flattened lookup and the ownership pool
            itemLookup[frn - 1] = h.get();      // Fast observer
            handlerOwnership.push_back(std::move(h)); // Owner
        }

        template <typename... HandlerTypes>
        void registerBatch() {
            // Fold expression expands to addHandler(...) for every type in HandlerTypes
            (addHandler(std::make_unique<HandlerTypes>(), HandlerTypes::FRN), ...);
        }

        /**
         * @brief Pointer to central diagnostic stats.
         */
        AsterixStats* stats_ptr = nullptr;

        /**
         * @brief Maximum Field Record Number supported in the flat array.
         * 128 covers all standard ASTERIX categories (max ~70-80 FRNs).
         */
        static constexpr size_t MAX_FRNS = 128;

        /**
         * @brief FLATTENED: O(1) Raw pointer lookup table.
         *
         * The index of the vector directly corresponds to the Field Record Number (FRN - 1),
         * ensuring fast lookups and a memory-efficient structure.
         */
        std::array<IAsterixDataItemHandler<T>*, MAX_FRNS> itemLookup{};

        /**
         * @brief OWNERSHIP: Manages the lifetime of all data item handlers.
         * Keeping these in a vector often results in them being allocated
         * contiguously in heap memory.
         */
        std::vector<std::unique_ptr<IAsterixDataItemHandler<T>>> handlerOwnership;

        /**
         * @brief Internal method that handles the F-spec parsing and data decoding.
         *
         * This method iterates through the F-spec bytes, identifies the presence of
         * data items, and dispatches the decoding task to the corresponding handler.
         * It returns the total number of bytes consumed from the data payload.
         *
         * @param fspec A pointer to the start of the Field Specification.
         * @param fspecSize The size of the F-spec in bytes.
         * @param data A pointer to the start of the data payload.
         * @param dataLeft The remaining size of the data payload in bytes.
         * @param context A reference to the context object (e.g., `Asterix1Report`, `AsterixNorth`)
         * to which the decoded data will be written by the individual item handlers.
         * @return size_t The total number of bytes consumed from the data payload.
         */
        [[nodiscard]]size_t _processDataRecordInternal(
                std::string_view fspec,
                std::string_view payload,
                T& context);
};

template <typename T>
size_t AsterixCategoryHandler<T>::_processDataRecordInternal(
        std::string_view fspec,
        std::string_view payload,
        T& context) {

    uint16_t frn_base = 1;

    std::string_view remainingData = payload;

    // Helper to log and exit
    auto abortWithStat = [&](std::atomic<uint64_t>& counter) -> size_t {
        if (stats_ptr) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
        return 0;
    };

    // 1. Validate Mandatory Fields
    if (fspec.size() < mandatoryFspecSize) [[unlikely]] {
        return abortWithStat(stats_ptr->protocolViolations);
    }

    // 2nd Check: Detailed bit-level comparison
    for (size_t i = 0; i < mandatoryFspecSize; ++i) {
        // (required & ~received) identifies mandatory bits NOT present in received F-spec.
        if (mandatoryFspec[i] & ~static_cast<uint8_t>(fspec[i])) [[unlikely]] {
            return abortWithStat(stats_ptr->protocolViolations);
        }
    }

    // Loop through each byte of the F-spec.
    for (const char c : fspec) {
        const uint8_t fspecByte = static_cast<uint8_t>(c);

        uint8_t itemBits = fspecByte & 0xFE; // Strip FX bit

        // Quick exit if no items in this byte
        while (itemBits) {
            // Get the index (0-6) of the highest set bit
            int offset = __builtin_clz(static_cast<uint32_t>(itemBits) << 24);
            uint16_t currentFrn = static_cast<uint16_t>(frn_base + offset);

            // Direct array access instead of vector lookup.
            // If FRN is within bounds, the CPU likely has this in the L1/L2 cache.
            // Get the handler first (nullptr if out of bounds or not registered)
            IAsterixDataItemHandler<T>* handler = itemLookup[currentFrn - 1];

            // BIT RAISED: We must decode this item
            if (handler) [[likely]] {
                // Determine item size and check buffer bounds.
                auto itemSize = handler->getSize(remainingData);
                if (itemSize == 0 || itemSize > remainingData.size()) {
                    // Not enough data was found in the payload for this item.
                    return abortWithStat(stats_ptr->malformedRecords);
                }

                // Decode the data into the context object and advance pointers.
                handler->decode(context, remainingData.substr(0, itemSize));

                remainingData.remove_prefix(itemSize);
            } else {
                // Update stats for missing decoder.
                return abortWithStat(stats_ptr->unhandledItems);
            }

            // Clear the bit we just processed to find the next one
            itemBits &= static_cast<uint8_t>(~(0x80 >> offset));
        }

        // If the FX bit (0x01) is NOT set, this is the last F-spec byte
        if (!(fspecByte & 0x01)) {
            return payload.size() - remainingData.size();
        }

        frn_base += 7;
    }

    // If we reach here, the loop finished but the last byte had FX=1
    return abortWithStat(stats_ptr->malformedRecords);
}

template <typename T>
void AsterixCategoryHandler<T>::setStats(AsterixStats& s) {
    this->stats_ptr = &s; // Store local pointer
    for (auto& handler : itemLookup) {
        if (handler) {
            handler->setStats(s); // Pass reference to sub-handlers
        }
    }
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
