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
class Asterix2Handler final
    : public AsterixCategoryHandler<Asterix2Report, IAsterix2Listener, Asterix2Handler> {
    public:
        // This line is essential for CRTP to work with private members:
        friend class AsterixCategoryHandler<Asterix2Report, IAsterix2Listener, Asterix2Handler>;

        using HandlerTypes = std::tuple<
            I002_010_Handler,
            I002_000_Handler,
            I002_020_Handler,
            I002_030_Handler,
            I002_041_Handler,
            I002_050_Handler
        >;

        static constexpr uint8_t Category = 2;

        // Define the F-Spec bitmasks as static constexpr
        // These are computed once at compile time based on the handler types
        static constexpr auto supportedFspec_ = FspecBuilder<HandlerTypes>::buildSupported();

        static constexpr auto mandatoryFspec_ = FspecBuilder<HandlerTypes>::buildMandatory();

        /**
         * @brief Constructor that initializes the data item handlers.
         */
        explicit Asterix2Handler(std::shared_ptr<SourceStateManager> manager);

        void setStats(AsterixStats& s) override;

    protected:
        // Implementation of the Hook: Time Synchronization Logic
        bool onAfterDecode(Asterix2Report& report, struct timespec ts);

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

        // --- Data Item Handlers (Statically Named) ---
        HandlerTypes m_handlers;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
