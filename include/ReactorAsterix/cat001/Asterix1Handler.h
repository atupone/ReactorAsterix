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
#include <ReactorAsterix/core/AsterixCategoryHandler.h>
#include <ReactorAsterix/cat001/Asterix1Report.h>

// System headers
#include <algorithm>
#include <vector>
#include <shared_mutex>
#include <mutex>

// Library headers
#include <ReactorAsterix/cat001/IAsterix1Listener.h>
#include <ReactorAsterix/core/SourceStateManager.h>

namespace ReactorAsterix {

/**
 * @class Asterix1Handler
 * @brief Handles ASTERIX Category 1: Monoradar Target Reports.
 */
class Asterix1Handler final : public AsterixCategoryHandler<Asterix1Report> {
    public:
        /**
         * @brief Constructor that initializes the data item handlers.
         */
        explicit Asterix1Handler(std::shared_ptr<SourceStateManager> manager);

        /**
         * @brief Adds a listener to the notification list.
         * Does not take ownership of the pointer. Duplicate listeners are ignored.
         */
        void addListener(IAsterix1Listener* l) {
            // EXCLUSIVE LOCK: Only one thread can write at a time
            std::unique_lock lock(listenerMutex);

            if (l && std::find(listeners.begin(), listeners.end(), l) == listeners.end()) {
                listeners.push_back(l);
            }
        }

        /**
         * @brief Main function for processing a single record.
         *
         * This function overrides the virtual method from `IAsterixCategoryHandler`.
         * It encapsulates the entire flow of decoding and forwarding the plot.
         *
         * @param fspec A pointer to the record's F-spec.
         * @param fspecSize The size of the F-spec.
         * @param data A pointer to the start of the payload.
         * @param dataLeft The remaining size of the payload.
         * @return size_t The total number of bytes consumed from the payload.
         */
        size_t processDataRecord(std::string_view fspec, std::string_view payload) override;

    protected:
        /**
         * @brief Registers the specific data item handlers for Category 1.
         *
         * This method is called by the constructor to populate the handlers map
         * with the correct decoding functions.
         */
        void registerHandlers() override;

    private:
        /**
         * @brief Pure logic helper: Expands 16-bit truncated TOD.
         * Static because it depends only on inputs, not object state.
         */
        static uint32_t expandTruncatedTime(uint16_t truncated, uint32_t reference) noexcept;

        /**
         * @brief Pure logic helper: Gets system time in ASTERIX units.
         */
        static uint32_t calculateCurrentTod() noexcept;

        // Supports multiple sinks (Logger, Tracker, Display)
        std::vector<IAsterix1Listener*> listeners;

        // C++17 Reader-Writer Lock
        mutable std::shared_mutex listenerMutex;

        std::shared_ptr<SourceStateManager> sourceStateManager;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
