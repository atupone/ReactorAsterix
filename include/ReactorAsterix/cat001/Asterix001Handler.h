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
#include <ReactorAsterix/cat001/Asterix001Report.h>

// Library headers
#include <ReactorAsterix/core/SourceStateManager.h>
#include <ReactorAsterix/cat001/IAsterix001Listener.h>
#include <ReactorAsterix/cat001/Asterix001DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @class Asterix001Handler
 * @brief Handles ASTERIX Category 1: Monoradar Target Reports.
 */
class Asterix001Handler final
    : public AsterixCategoryHandler<Asterix001Report, IAsterix001Listener, Asterix001Handler> {
        // This line is essential for CRTP to work with private members:
        friend class AsterixCategoryHandler<Asterix001Report, IAsterix001Listener, Asterix001Handler>;
    public:
        static constexpr uint8_t Category = 1;

        /**
         * @brief Constructor that initializes the data item handlers.
         */
        explicit Asterix001Handler(std::shared_ptr<SourceStateManager> manager);

    protected:
        // Implementation of the Hook: Time Synchronization Logic
        bool onAfterDecode(Asterix001Report& report, struct timespec ts);

    private:
        /**
         * @brief Pure logic helper: Expands 16-bit truncated TOD.
         * Static because it depends only on inputs, not object state.
         */
        static uint32_t expandTruncatedTime(uint16_t truncated, uint32_t reference) noexcept;

        /**
         * @brief Pure logic helper: Gets system time in ASTERIX units.
         */
        static uint32_t calculateCurrentTod(struct timespec ts) noexcept;

        // Supports multiple sinks (Logger, Tracker, Display)
        std::vector<std::weak_ptr<IAsterix001Listener>> listeners;

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
