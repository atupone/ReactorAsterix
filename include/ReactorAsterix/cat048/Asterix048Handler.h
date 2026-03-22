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
#include <ReactorAsterix/cat048/Asterix048Report.h>

// Library headers
#include <ReactorAsterix/core/SourceStateManager.h>
#include <ReactorAsterix/cat048/IAsterix048Listener.h>
#include <ReactorAsterix/cat048/Asterix048DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix048Handler
 * @brief Handles ASTERIX Category 048: Monoradar Target Reports.
 */
class Asterix048Handler final
    : public AsterixCategoryHandler<Asterix048Report, IAsterix048Listener, Asterix048Handler> {
        // This line is essential for CRTP to work with private members:
        friend class AsterixCategoryHandler<Asterix048Report, IAsterix048Listener, Asterix048Handler>;
    public:
        /**
         * @brief Constructor that initializes the data item handlers.
         */
        explicit Asterix048Handler(std::shared_ptr<SourceStateManager> manager);

    protected:
        // Implementation of the Hook: Time Synchronization Logic
        void onAfterDecode(struct timespec ts);

        // Supports multiple sinks (Logger, Tracker, Display)
        std::vector<std::weak_ptr<IAsterix048Listener>> listeners;

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
