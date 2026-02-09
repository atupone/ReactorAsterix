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

// System headers
#include <cstddef>
#include <cstdint>

// Library headers
#include <ReactorAsterix/core/SourceIdentifier.h>
#include <ReactorAsterix/core/SourceStateManager.h>

namespace ReactorAsterix {

/**
 * @class AsterixMessage
 * @brief The base class for all decoded ASTERIX data.
 * focusing purely on shared metadata like Source ID and Reception Time.
 */
class AsterixMessage {
    public:
        AsterixMessage() = default;
        virtual ~AsterixMessage() = default;

        void reset();

        // Temporary link to the manager, set by the Handler at creation
        SourceStateManager* manager = nullptr;

        // Pointer to the persistent sensor state in the SourceStateManager deque
        const SourceRecord* sourceRecord = nullptr;

        // Uniquely identifies the radar station
        SourceIdentifier sourceIdentifier;

        // The time the message was received
        uint32_t TOD;

        // Reusable setter used by Ixxx/010 Handlers across all categories
        void setSourceIdentifier(uint8_t sac, uint8_t sic) {
            sourceIdentifier = {sac, sic};

            // If the handler provided a manager, we "hook" into the persistent state
            if (manager) {
                // Use a combined 16-bit key for the fastest possible comparison
                uint16_t currentKey = (static_cast<uint16_t>(sac) << 8) | sic;

                // Define the static thread_local cache within the method
                static thread_local uint16_t lastKey
                    __attribute__((tls_model("initial-exec"))) = 0;
                static thread_local const SourceRecord* cachedRecord
                    __attribute__((tls_model("initial-exec"))) = nullptr;

                if (currentKey == lastKey && cachedRecord) [[likely]] {
                    // SUCCESS: Reuse the record from the previous message (any category)
                    sourceRecord = cachedRecord;
                } else {
                    // MISS: Different radar or first time. Perform the expensive lock.
                    // This retrieves the stable pointer from the deque
                    sourceRecord = manager->getOrCreateRecord(sourceIdentifier);

                    // Update the cache for the next message in this thread
                    cachedRecord = sourceRecord;
                    lastKey = currentKey;
                }
            }
        }
};

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
