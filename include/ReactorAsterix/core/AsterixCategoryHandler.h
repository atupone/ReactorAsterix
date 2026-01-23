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
#include <mutex>
#include <vector>

// Libray headers
#include <ReactorAsterix/core/IAsterixDataItemHandler.h>
#include <ReactorAsterix/core/AsterixDiagnostics.h>

namespace ReactorAsterix {

// -----------------------------------------------------------------------
// Helper: Linear search using C++17 Fold Expression
// This avoids recursive template instantiation limits and errors.
// -----------------------------------------------------------------------
template <typename Tuple, uint8_t TargetFRN, size_t... Is>
    constexpr int findHandlerIndexImpl(std::index_sequence<Is...>) {
        int found = -1;
        // Unfold checks across all tuple elements
        ( ( (std::tuple_element_t<Is, Tuple>::FRN == TargetFRN) ? (found = static_cast<int>(Is)) : 0 ), ... );
        return found;
    }

// -----------------------------------------------------------------------
// Replacement HandlerIndexFinder
// Keeps your existing API (::value) but uses the robust implementation.
// -----------------------------------------------------------------------
template <typename Tuple, uint8_t TargetFRN>
struct HandlerIndexFinder {
    static constexpr int value = findHandlerIndexImpl<Tuple, TargetFRN>(
        std::make_index_sequence<std::tuple_size_v<Tuple>>{}
    );
};

/**
 * @brief Helper to handle the "Boilerplate" of getting size and decoding.
 * Making this a template ensures the compiler knows the EXACT type of 'H' and 'R'.
 */
template<typename H, typename R>
    bool execute(H& handler, R& report, std::string_view& data) {
        // Determine item size and check buffer bounds.
        auto itemSize = handler.getSize(data);
        if (itemSize > data.size()) [[unlikely]] {
            // Not enough data was found in the payload for this item.
            return false;
        }

        // Decode the data into the context object and advance pointers.
        handler.decode(report, data.substr(0, itemSize));

        data.remove_prefix(itemSize);
        return true;
    }

/**
 * @class AsterixCategoryHandler
 * @brief Base template for specific category handlers (e.g., Cat 001).
 * @tparam T The Record type (context) this handler populates.
 */
template <typename T, typename ListenerInterface>
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

        using ListenerList = std::vector<std::weak_ptr<ListenerInterface>>;

        void addListener(std::shared_ptr<ListenerInterface> l) {
            if (!l) return;

            // Mutex only blocks other writers (addListener/removeListener)
            std::lock_guard<std::mutex> lock(writeMutex_);

            // Load current list
            auto current = std::atomic_load(&listeners_);

            // Create a copy (COW)
            auto next = std::make_shared<ListenerList>(*current);

            // Clean up expired while adding new
            next->erase(std::remove_if(next->begin(), next->end(),
                [](const std::weak_ptr<ListenerInterface>& wp) { return wp.expired(); }),
                next->end());

            // Add new listener
            next->push_back(l);

            // Atomic store
            std::atomic_store(&listeners_, next);
        }

        void removeListener(std::shared_ptr<ListenerInterface> l) {
            if (!l) return;
            std::lock_guard<std::mutex> lock(writeMutex_);

            auto current = std::atomic_load(&listeners_);
            auto next = std::make_shared<ListenerList>(*current);

            next->erase(std::remove_if(next->begin(), next->end(),
                [&l](const std::weak_ptr<ListenerInterface>& wp) {
                    auto sp = wp.lock();
                    return !sp || sp == l;
                }), next->end());

            std::atomic_store(&listeners_, next);
        }

    protected:
        // Pre-computed F-spec where bits are 1 if the item is mandatory
        std::array<uint8_t, 20> mandatoryFspec{};
        size_t mandatoryFspecSize = 0; // Tracks the highest byte index used

        // ASTERIX FSPECs can be multiple bytes. A 20-byte array covers most categories.
        std::array<uint8_t, 20> supportedFspec = {0};

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

            // Determine which byte and bit in the F-spec this FRN corresponds to
            const size_t uFrn = static_cast<size_t>(frn);
            size_t byteIdx = (uFrn - 1) / 7;
            size_t bitIdx  = 7 - ((uFrn - 1) % 7); // Bits 7 to 1

            if (byteIdx < supportedFspec.size()) {
                supportedFspec[byteIdx] |= static_cast<uint8_t>(1 << bitIdx);
            }

            if (h->isMandatory()) {
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

        /**
         * @brief Checks if any bit is set in the received FSPEC for which
         * we do NOT have a registered handler.
         */
        bool checkAllHandlersSupported(std::string_view fspec) const {
            // Only check up to the size of the received FSPEC
            for (size_t i = 0; i < fspec.size(); ++i) {
                uint8_t received = static_cast<uint8_t>(fspec[i]);

                // If the received FSPEC is longer than our mask, any bit set
                // in the extra bytes is unsupported by definition.
                uint8_t supported = (i < supportedFspec.size()) ? supportedFspec[i] : 0;

                // (received & ~supported) identifies bits set that we don't support.
                // We & with 0xFE to ignore the FX (extension) bit.
                if ((received & ~supported) & 0xFE) {
                    return false;
                }
            }
            return true;
        }

        // Helper to unroll a single octet (7 data bits + 1 FX bit)
        template <size_t StartIdx, size_t... Is, typename HandlerTuple, typename Report>
        static void unroll_octet(uint8_t octet, std::index_sequence<Is...>,
                HandlerTuple& handlers, Report& report, std::string_view& data) {

            ([&] {
                // Calculate the actual FRN represented by this bit
                constexpr uint8_t CurrentFRN = static_cast<uint8_t>(StartIdx + Is + 1);

                // COMPILE-TIME LOOKUP: Find the correct handler index for this FRN
                constexpr int TupleIdx = HandlerIndexFinder<HandlerTuple, CurrentFRN>::value;

                // Only generate code if we actually have a handler for this FRN
                if constexpr (TupleIdx != -1) {
                    // Check if the bit is set in the FSPEC
                    if (octet & (0x80 >> Is)) {
                        std::get<TupleIdx>(handlers).decode(report, data);
                    }
                }
             }(), ...);
        }

        // Main entry point for the unrolled decoding logic
        template <size_t OctetIdx = 0, typename HandlerTuple, typename Report>
            static void process_all_octets_unrolled(std::string_view fspec, std::string_view& data,
                    HandlerTuple& handlers, Report& report) {
                if (OctetIdx >= fspec.size()) return;

                uint8_t octet = static_cast<uint8_t>(fspec[OctetIdx]);

                // Unroll bits for the current octet index
                unroll_octet<OctetIdx * 7>(octet, std::make_index_sequence<7>{}, handlers, report, data);

                // If FX bit is set and the next octet has potential handlers, recurse
                if constexpr ((OctetIdx + 1) * 7 < std::tuple_size_v<HandlerTuple>) {
                    if (octet & 0x01) {
                        process_all_octets_unrolled<OctetIdx + 1>(fspec, data, handlers, report);
                    }
                }
            }

        /**
         * @brief The shared internal logic for all ASTERIX categories.
         */
        template <typename ReportType, typename HandlerTuple>
            size_t processDataRecordGeneric(
                    std::string_view fspec,
                    std::string_view payload,
                    ReportType& context,
                    HandlerTuple& handlers)
            {
                // Validation logic
                if (!checkMandatoryItems(fspec)) [[unlikely]] {
                    if (stats_ptr) stats_ptr->protocolViolations.fetch_add(1, std::memory_order_relaxed);
                    return 0;
                }

                if (!checkAllHandlersSupported(fspec)) [[unlikely]] {
                    if (stats_ptr) stats_ptr->unhandledItems.fetch_add(1, std::memory_order_relaxed);
                    return 0;
                }

                std::string_view data = payload;

                // Execute the unrolled decoding
                process_all_octets_unrolled(fspec, data, handlers, context);

                size_t consumed = payload.size() - data.size();

                if (consumed == 0 && payload.size() > 0) [[unlikely]] {
                    if (stats_ptr) stats_ptr->malformedRecords.fetch_add(1, std::memory_order_relaxed);
                    return 0;
                }

                return consumed;
            }

        template <typename... HandlerTypes>
        void registerBatch() {
            // Fold expression expands to addHandler(...) for every type in HandlerTypes
            (addHandler(std::make_unique<HandlerTypes>(), HandlerTypes::FRN), ...);
        }

        template <typename ReportType, typename HandlersTuple>
            bool dispatch(int frn, ReportType& report, std::string_view& data, HandlersTuple& handlers) {
                return std::apply([&](auto&... h) {
                    // Fold expression: ((condition ? execute(...) : false) || ...)
                    // Expands at compile time to a sequence of checks.
                    return ((h.FRN == frn ? execute(h, report, data) : false) || ...);
                }, handlers);
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

        [[nodiscard]]bool checkMandatoryItems(std::string_view fspec);

        // This allows derived classes (Cat001, Cat002) to get the snapshot
        std::shared_ptr<ListenerList> getListeners() const {
            return std::atomic_load(&listeners_);
        }

    private:
        std::shared_ptr<ListenerList> listeners_{std::make_shared<ListenerList>()};
        std::mutex writeMutex_; // Only for writers (addListener)
};

template <typename T, typename ListenerInterface>
bool AsterixCategoryHandler<T, ListenerInterface>::checkMandatoryItems(std::string_view fspec) {

    // 1. Validate Mandatory Fields
    if (fspec.size() < mandatoryFspecSize) [[unlikely]] {
        return false;
    }

    // 2nd Check: Detailed bit-level comparison
    for (size_t i = 0; i < mandatoryFspecSize; ++i) {
        // (required & ~received) identifies mandatory bits NOT present in received F-spec.
        if (mandatoryFspec[i] & ~static_cast<uint8_t>(fspec[i])) [[unlikely]] {
            return false;
        }
    }
    return true;
}

template <typename T, typename ListenerInterface>
void AsterixCategoryHandler<T, ListenerInterface>::setStats(AsterixStats& s) {
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
