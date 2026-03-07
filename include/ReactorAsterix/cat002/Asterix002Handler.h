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
#include <ReactorAsterix/cat002/Asterix002Report.h>

// Library headers
#include <ReactorAsterix/core/SourceStateManager.h>
#include <ReactorAsterix/cat002/IAsterix002Listener.h>
#include <ReactorAsterix/cat002/Asterix002DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix002Handler
 * @brief Handles ASTERIX Category 2: Monoradar Service Messages.
 */
class Asterix002Handler final
    : public AsterixCategoryHandler<Asterix002Report, IAsterix002Listener, Asterix002Handler> {
    public:
        // This line is essential for CRTP to work with private members:
        friend class AsterixCategoryHandler<Asterix002Report, IAsterix002Listener, Asterix002Handler>;

        static constexpr uint8_t Category = 2;

        /**
         * @brief Constructor that initializes the data item handlers.
         */
        explicit Asterix002Handler(std::shared_ptr<SourceStateManager> manager);

    protected:
        // Implementation of the Hook: Time Synchronization Logic
        void onAfterDecode(struct timespec ts);

    private:
        // Supports multiple sinks (Logger, Tracker, Display)
        std::vector<std::weak_ptr<IAsterix002Listener>> listeners;

        // C++17 Reader-Writer Lock
        mutable std::shared_mutex listenerMutex;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
