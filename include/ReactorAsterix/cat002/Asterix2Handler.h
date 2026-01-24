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
#include <ReactorAsterix/cat002/Asterix2Report.h>

// Library headers
#include <ReactorAsterix/core/SourceStateManager.h>
#include <ReactorAsterix/cat002/IAsterix2Listener.h>
#include <ReactorAsterix/cat002/Asterix2DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix2Handler
 * @brief Handles ASTERIX Category 2: Monoradar Service Messages.
 */
class Asterix2Handler final : public AsterixCategoryHandler<Asterix2Report, IAsterix2Listener> {
    public:
        /**
         * @brief Constructor that initializes the data item handlers.
         */
        explicit Asterix2Handler(std::shared_ptr<SourceStateManager> manager);

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
        size_t processDataRecord(
                std::string_view fspec,
                std::string_view payload,
                struct timespec ts) override;

        void setStats(AsterixStats& s) override;
    protected:
        /**
         * @brief Registers the specific data item handlers for Category 2.
         *
         * This method is called by the constructor to populate the handlers map
         * with the correct decoding functions.
         */
        void registerHandlers() override;

    private:
        /**
         * @brief This is the "Hot Path".
         * Because we call methods on named members, the compiler
         * will likely INLINE these calls.
         */
        bool dispatch(int frn, Asterix2Report& report, std::string_view& data);

        // Supports multiple sinks (Logger, Tracker, Display)
        std::vector<std::weak_ptr<IAsterix2Listener>> listeners;

        // C++17 Reader-Writer Lock
        mutable std::shared_mutex listenerMutex;

        std::shared_ptr<SourceStateManager> sourceStateManager;

        // --- Data Item Handlers (Statically Named) ---
        std::tuple<
            I002_010_Handler,
            I002_000_Handler,
            I002_020_Handler,
            I002_030_Handler,
            I002_041_Handler,
            I002_050_Handler
        > m_handlers;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
