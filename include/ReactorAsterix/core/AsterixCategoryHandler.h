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
#include <memory>
#include <mutex>
#include <vector>

// Libray headers
#include <ReactorAsterix/core/AsterixDiagnostics.h>
#include <ReactorAsterix/core/SourceStateManager.h>

namespace ReactorAsterix {

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
            report.reset();

            // Common Logic Hoisted: Every report gets the manager reference
            // This assumes 'T' (the report) has a 'manager' field
            // Unified Assignment: Works for all Categories
            report.manager = sourceStateManager.get();

            std::string_view data = payload;

            bool result = report.process_all_octets(fspec, data, localStats, *this);

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
            bool keep_report = static_cast<Derived*>(this)->onAfterDecode(ts);

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

        /**
         * High-speed member-based cache to replace TLS
         */
        const SourceRecord* getSourceRecordCached(uint8_t sac, uint8_t sic) {
            uint16_t currentKey = (static_cast<uint16_t>(sac) << 8) | sic;

            // Direct comparison - no null checks, no TLS, just raw speed
            if (currentKey == m_lastCacheKey) [[likely]] {
                return m_cachedRecord;
            }

            // Only hit the shared_ptr and manager on a cache miss (rare)
            m_cachedRecord = sourceStateManager->getOrCreateRecord({sac, sic});
            m_lastCacheKey = currentKey;
            return m_cachedRecord;
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

        T report; // Persistent member variable, no TLS lookup needed

        std::shared_ptr<SourceStateManager> sourceStateManager;

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
