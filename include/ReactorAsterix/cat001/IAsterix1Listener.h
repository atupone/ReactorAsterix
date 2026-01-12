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

namespace ReactorAsterix {

class Asterix1Report;

/**
 * @class IAsterix1Listener
 * @brief High-performance interface for receiving decoded Category 1 reports.
 */
class IAsterix1Listener {
    public:
        virtual ~IAsterix1Listener() = default;

        /**
         * @brief Called by the handler when a record is successfully decoded.
         * Uses a virtual call which is faster than std::function for shared libraries.
         */
        virtual void onReportDecoded(std::shared_ptr<Asterix1Report> report) = 0;
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
