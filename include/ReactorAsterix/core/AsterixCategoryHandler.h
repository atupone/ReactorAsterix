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
#include <iostream>

// Libray headers
#include <ReactorAsterix/core/IAsterixDataItemHandler.h>
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/core/AsterixMessage.h>

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

// Define a trait to check mandatory status at compile time
template<typename T>
struct is_mandatory { static constexpr bool value = false; };

template <typename Tuple>
struct FspecBuilder;

// Compute F-Specs at Compile Time
template<typename... Handlers>
constexpr std::array<uint8_t, 20> buildSupportedFspec() {
    std::array<uint8_t, 20> fspec{};
    // Fold expression to set bits for each handler
    ((fspec[(Handlers::FRN - 1) / 7] |= (1 << (7 - ((Handlers::FRN - 1) % 7)))), ...);
    return fspec;
}

template <typename... Handlers>
struct FspecBuilder<std::tuple<Handlers...>> {
    // Builds mask for ALL supported handlers
    static constexpr auto buildSupported() {
        // This calls your existing buildSupportedFspec using the unpacked types
        return buildSupportedFspec<Handlers...>();
    }

    // Builds mask ONLY for handlers where 'mandatory' is true
    static constexpr auto buildMandatory() {
        std::array<uint8_t, 20> mask = {0};

        // Use a fold expression to check each handler's static 'mandatory' property
        // Note: You need to add 'static constexpr bool mandatory' to your handler classes
        ((Handlers::mandatory ? applyBit(mask, Handlers::FRN) : void()), ...);

        return mask;
    }

    private:
        static constexpr void applyBit(std::array<uint8_t, 20>& mask, uint8_t frn) {
            size_t byteIndex = static_cast<size_t>((frn - 1) / 7);
            int bitIndex  = 7 - ((frn - 1) % 7);
            mask[byteIndex] |= static_cast<uint8_t>(1 << bitIndex);
        }
};

template<typename... Handlers>
constexpr std::array<uint8_t, 20> buildMandatoryFspec() {
    std::array<uint8_t, 20> fspec{};
    // Only set bit if Handler::is_mandatory is true
    ((Handlers::is_mandatory ?
      (fspec[(Handlers::FRN - 1) / 7] |= (1 << (7 - ((Handlers::FRN - 1) % 7)))) : 0), ...);
    return fspec;
}

/**
 * @class AsterixCategoryHandler
 * @brief Base template for specific category handlers (e.g., Cat 001).
 * @tparam T The Record type (context) this handler populates.
 */
template <typename T, typename ListenerInterface, typename Derived>
class AsterixCategoryHandler : public IAsterixCategoryHandler {
    public:
        explicit AsterixCategoryHandler(std::shared_ptr<SourceStateManager> manager)
            : sourceStateManager(std::move(manager)) {}
        /**
         * @brief Virtual destructor to ensure proper cleanup of derived classes
         * and managed handlers.
         */
        virtual ~AsterixCategoryHandler() = default;

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

        /**
         * @brief The Hoisted Orchestrator.
         * Shared by all categories (Cat 001, 002, etc.)
         */
        size_t processDataRecord(std::string_view fspec, std::string_view payload, struct timespec ts) override {
            // Initialize Report (Type T is Asterix1Report, Asterix2Report, etc.)
            T report;

            // Common Logic Hoisted: Every report gets the manager reference
            // This assumes 'T' (the report) has a 'manager' field
            // Unified Assignment: Works for all Categories
            report.manager = sourceStateManager.get();

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
            // Note: we pass the m_handlers from the Derived class
            process_all_octets_unrolled(
                fspec,
                data,
                static_cast<Derived*>(this)->m_handlers,
                report,
                *stats_ptr);

            size_t consumed = payload.size() - data.size();

            if (consumed == 0 && payload.size() > 0) [[unlikely]] {
                if (stats_ptr) stats_ptr->malformedRecords.fetch_add(1, std::memory_order_relaxed);
                return 0;
            }

            // Malformed or empty
            if (consumed <= 0) [[unlikely]] {
                return 0;
            }

            // Missing Source State
            if (!report.sourceRecord) [[unlikely]] {
                return consumed; // We decoded items, but can't sync time
            }

            // Hook: Post-decode logic (TOD synchronization, midnight wrap-around)
            bool keep_report = static_cast<Derived*>(this)->onAfterDecode(report, ts);

            if (keep_report) [[likely]] {
                // Notify Listeners (Shared logic)
                // LOCK-FREE NOTIFICATION
                // Just take a reference to the current list.
                // Even if a writer updates 'listeners' now, we safely iterate our local 'snapshot'.
                auto currentListeners = this->getListeners();

                for (auto const& weak_l : *currentListeners) {
                    if (auto l = weak_l.lock()) {
                        l->onReportDecoded(report);
                    }
                }
            }

            return consumed;
        }

        // Default hook (can be overridden by Derived if needed)
        bool onAfterDecode(T& /*report*/, struct timespec /*ts*/) {
            return true;
        }

    protected:
        /**
         * @brief Checks if any bit is set in the received FSPEC for which
         * we do NOT have a registered handler.
         */
        bool checkAllHandlersSupported(std::string_view fspec) const {
            // Access the static constant from the specific subclass (e.g., Asterix1Handler)
            const auto& supported = Derived::supportedFspec_;

            for (size_t i = 0; i < fspec.size(); ++i) {
                uint8_t received = static_cast<uint8_t>(fspec[i]);

                // If the received FSPEC is longer than our mask, any bit set
                // in the extra bytes is unsupported by definition.
                uint8_t mask = (i < supported.size()) ? supported[i] : 0;

                // bits set in received but NOT in supported mask
                uint8_t unsupportedBits = (received & ~mask) & 0xFE;

                // (received & ~mask) identifies bits set that we don't support.
                // We & with 0xFE to ignore the FX (extension) bit.
                if (unsupportedBits) {
                    // Logic to identify the FRN:
                    for (uint8_t bit = 0; bit < 7; ++bit) {
                        if (unsupportedBits & (0x80 >> bit)) {
                            size_t missingFrn = (i * 7) + static_cast<size_t>(bit + 1);
                            std::cerr << "[Asterix] Category " << (int)Derived::Category
                                << " - Missing handler for FRN " << missingFrn << std::endl;
                        }
                    }
                    return false;
                }
            }
            return true;
        }

        template<size_t StartFRN, size_t... Is, typename HandlerTuple, typename Report>
        static bool unroll_octet_impl(uint8_t octet, std::index_sequence<Is...>, HandlerTuple& handlers,
                Report& report, std::string_view& data, AsterixStats& stats) {
            bool success = true;

            // Fold expression: iterates over the pack 0, 1, 2, 3, 4, 5, 6
            ([&] {
                if (!success) return; // Already failed, skip this bit

                constexpr size_t CurrentFRN = StartFRN + Is;

                // Check if the bit for this FRN is set (Bit 7 down to Bit 1)
                if (octet & (0x80 >> Is)) {
                    // Find the index in your handlers tuple
                    constexpr int TupleIdx = HandlerIndexFinder<HandlerTuple,
                                                                static_cast<unsigned char>(CurrentFRN)>::value;

                    if constexpr (TupleIdx != -1) {
                        // Call your 'execute' helper!
                        // This does the getSize check, decode, and remove_prefix.
                        if (!execute(std::get<TupleIdx>(handlers), report, data)) {
                            stats.malformedRecords.fetch_add(1, std::memory_order_relaxed);
                            success = false; // Set failure flag
                        }
                    }
                }
             }(), ...);

            return success;
        }

        template<size_t StartFRN, typename HandlerTuple, typename Report>
        static bool unroll_octet(uint8_t octet, HandlerTuple& handlers,
                Report& report, std::string_view& data, AsterixStats& stats) {
            return unroll_octet_impl<StartFRN>(
                    octet, std::make_index_sequence<7>{}, handlers, report, data, stats);
        }

        template <size_t Threshold, typename Tuple, size_t... Is>
        static constexpr bool HasHandlersBeyondHelper(std::index_sequence<Is...>) {
            return ((std::tuple_element_t<Is, Tuple>::FRN > Threshold) || ...);
        }

        template <size_t Threshold, typename Tuple>
        static constexpr bool HasHandlersBeyond() {
            return HasHandlersBeyondHelper<Threshold, Tuple>(
                std::make_index_sequence<std::tuple_size_v<Tuple>>{}
            );
        }

        // Main entry point for the unrolled decoding logic
        template <size_t OctetIdx = 0, typename HandlerTuple, typename Report>
        static bool process_all_octets_unrolled(std::string_view fspec, std::string_view& data,
                HandlerTuple& handlers, Report& report, AsterixStats& stats) {
            // Runtime safety: Don't read past the actual F-Spec received
            if (OctetIdx >= fspec.size()) return true;

            uint8_t octet = static_cast<uint8_t>(fspec[OctetIdx]);

            // Calculate which FRN this octet starts with
            constexpr size_t StartFRN = (OctetIdx * 7) + 1;

            // We ALWAYS unroll 7 bits because any of them could match a handler in the sparse tuple
            if (!unroll_octet<StartFRN>(octet, handlers, report, data, stats)) {
                return false;
            }

            // Recursion logic:
            // We only stop recursing if we are sure NO handler in the tuple
            // has an FRN higher than what we just processed.
            if constexpr (HasHandlersBeyond<StartFRN + 6, HandlerTuple>()) {
                if (octet & 0x01) {
                    return process_all_octets_unrolled<OctetIdx + 1>(fspec, data, handlers, report, stats);
                }
            }

            return true;
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

        std::shared_ptr<SourceStateManager> sourceStateManager;

        /**
         * @brief Maximum Field Record Number supported in the flat array.
         * 128 covers all standard ASTERIX categories (max ~70-80 FRNs).
         */
        static constexpr size_t MAX_FRNS = 128;

        [[nodiscard]]bool checkMandatoryItems(std::string_view fspec) const {
            // Access the constant from the specific subclass (e.g., Asterix1Handler)
            const auto& mandatory = Derived::mandatoryFspec_;

            const size_t bytesToCheck = std::min(fspec.size(), mandatory.size());
            for (size_t i = 0; i < bytesToCheck; ++i) {
                if ((static_cast<uint8_t>(fspec[i]) & mandatory[i]) != mandatory[i]) {
                    return false;
                }
            }

            if (fspec.size() < mandatory.size()) {
                for (size_t i = fspec.size(); i < mandatory.size(); ++i) {
                    if (mandatory[i] != 0) {
                        return false; // A mandatory bit exists beyond the provided FSPEC
                    }
                }
            }

            return true;
        };

        // This allows derived classes (Cat001, Cat002) to get the snapshot
        std::shared_ptr<ListenerList> getListeners() const {
            return std::atomic_load(&listeners_);
        }

    private:
        std::shared_ptr<ListenerList> listeners_{std::make_shared<ListenerList>()};
        std::mutex writeMutex_; // Only for writers (addListener)
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
