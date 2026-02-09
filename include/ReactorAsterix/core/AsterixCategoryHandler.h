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
        size_t processDataRecord(
                std::string_view fspec,
                std::string_view payload,
                struct timespec ts,
                AsterixStatsData& localStats) override {
            // Initialize Report (Type T is Asterix1Report, Asterix2Report, etc.)
            T report;

            // Common Logic Hoisted: Every report gets the manager reference
            // This assumes 'T' (the report) has a 'manager' field
            // Unified Assignment: Works for all Categories
            report.manager = sourceStateManager.get();

            // Validation logic
            if (!checkAllHandlersSupported(fspec)) [[unlikely]] {
                localStats.unhandledItems++;
                return 0;
            }

            std::string_view data = payload;

            bool result = report.process_all_octets(fspec, data, localStats);

            if (!result) [[unlikely]] {
                return 0;
            }

            size_t consumed = payload.size() - data.size();

            if (consumed == 0 && payload.size() > 0) [[unlikely]] {
                localStats.malformedRecords++;
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
         * @brief Applies the source offset and handles midnight wrap-around.
         * Transitions a raw Radar TOD to the System Domain.
         */
        uint32_t applyTimeCorrection(uint32_t rawTod, const SourceRecord& record) const {
            // Update the Radar's 24h clock state for reference
            record.lastTod.store(rawTod, std::memory_order_relaxed);

            // Apply Offset (Transition to System Domain)
            constexpr int32_t TICKS_PER_DAY = 86400 * 128;
            int32_t corrected = static_cast<int32_t>(rawTod)
                - record.averageOffset.load(std::memory_order_relaxed);

            if (corrected < 0) {
                corrected += TICKS_PER_DAY;
            } else if (corrected >= TICKS_PER_DAY) {
                corrected -= TICKS_PER_DAY;
            }

            return static_cast<uint32_t>(corrected);
        }

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
