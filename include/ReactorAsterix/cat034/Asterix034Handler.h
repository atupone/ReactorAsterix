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
#include <ReactorAsterix/cat034/Asterix034Report.h>

// Library headers
#include <ReactorAsterix/core/SourceStateManager.h>
#include <ReactorAsterix/cat034/IAsterix034Listener.h>
#include <ReactorAsterix/cat034/Asterix034DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix034Handler
 * @brief Handles ASTERIX Category 034: Monoradar Service Messages.
 */
class Asterix034Handler final
    : public AsterixCategoryHandler<Asterix034Report, IAsterix034Listener, Asterix034Handler> {
    public:
        // This line is essential for CRTP to work with private members:
        friend class AsterixCategoryHandler<Asterix034Report, IAsterix034Listener, Asterix034Handler>;

        using HandlerTypes = std::tuple<
            I034_010_Handler,
            I034_000_Handler,
            I034_030_Handler,
            I034_020_Handler,
            I034_041_Handler,
            I034_120_Handler
        >;

        static constexpr uint8_t Category = 34;

        // Define the F-Spec bitmasks as static constexpr
        // These are computed once at compile time based on the handler types
        static constexpr auto supportedFspec_ = FspecBuilder<HandlerTypes>::buildSupported();

        static constexpr auto mandatoryFspec_ = FspecBuilder<HandlerTypes>::buildMandatory();

        /**
         * @brief Constructor that initializes the data item handlers.
         */
        explicit Asterix034Handler(std::shared_ptr<SourceStateManager> manager);

    protected:
        // Implementation of the Hook: Time Synchronization Logic
        bool onAfterDecode(Asterix034Report& report, struct timespec ts);

    private:
        // Supports multiple sinks (Logger, Tracker, Display)
        std::vector<std::weak_ptr<IAsterix034Listener>> listeners;

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
