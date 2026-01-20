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

// Own header
#include <ReactorAsterix/cat002/Asterix2Handler.h>

// Library headers
#include <ReactorAsterix/core/AsterixConstants.h>
#include <ReactorAsterix/cat002/Asterix2DataItemCollection.h>

namespace ReactorAsterix {

using namespace Constants;

/**
 * @brief Constructor for the ASTERIX Category 2 Handler.
 */
Asterix2Handler::Asterix2Handler(std::shared_ptr<SourceStateManager> manager)
    : sourceStateManager(manager) {
    registerHandlers();
}

/**
 * @brief Registers the specific handlers for ASTERIX Category 2 data items.
 *
 * This method overrides the pure virtual method from the base class. It populates
 * the `handlers` vector with unique pointers to the concrete handler classes,
 * mapping each handler to its corresponding Field Record Number (FRN).
 */
void Asterix2Handler::registerHandlers() {
    // Register handlers at index = FRN - 1.
    registerBatch<
    I002_010_Handler, // I002/010: Data Source Identifier
    I002_000_Handler, // I002/000: Message Type
    I002_020_Handler, // I002/020: Sector Number
    I002_030_Handler, // I002/030: Time of Day
    I002_041_Handler, // I002/041: Antenna Rotation Speed
    I002_050_Handler  // I002/050: Station Configuration Status
    >();
}

void Asterix2Handler::addListener(std::shared_ptr<IAsterix2Listener> l) {
    if (!l) return;

    // EXCLUSIVE LOCK: Only one thread can write at a time
    std::unique_lock lock(listenerMutex);

    // Check if the listener is already present to avoid duplicates
    auto it = std::find_if(listeners.begin(), listeners.end(),
        [&l](const std::weak_ptr<IAsterix2Listener>& existing) {
            // lock() gets a shared_ptr; we compare it to our target 'l'
            return existing.lock() == l;
        });

    if (it == listeners.end()) {
        listeners.push_back(l);
    }
}

/**
 * @brief Handles the processing of a single ASTERIX Category 2 data record.
 *
 * This function is the entry point for processing a North message. It creates an `Asterix2Repor`
 * object, uses the base class's internal decoding mechanism, and then uses the decoded
 * Time of Day (TOD) for tracking synchronization.
 *
 * @param fspec A pointer to the start of the Field Specification.
 * @param fspecSize The size of the F-spec in bytes.
 * @param data A pointer to the start of the data payload.
 * @param dataLeft The remaining size of the data payload in bytes.
 * @return size_t The total number of bytes consumed by this handler.
 */
size_t Asterix2Handler::processDataRecord(
        std::string_view fspec,
        std::string_view payload,
        struct timespec ts)
{
    // Create the context object (Asterix2Report).
    Asterix2Report report;

    // Decode everything first.
    size_t consumed = this->_processDataRecordInternal(fspec, payload, report);

    if (consumed == 0 && sourceStateManager) {
        // ALWAYS update the Radar's 24h clock state (for bit-stitching ref)
        sourceStateManager->updateSourceTime(report.sourceIdentifier, report.TOD);

        // Timing Synchronization Logic: Only triggered by North Marker (Type 1)
        if (report.messageType == Asterix2Report::MessageType::NORTH_MARKER) {

            // Get seconds since midnight using simple modulo
            // 86400 seconds in a day
            uint32_t seconds_since_midnight = static_cast<uint32_t>(ts.tv_sec % 86400);

            // Conversion using the exact integer divisor
            uint32_t kernel_128th = (seconds_since_midnight * AST_TOD_UNITS_PER_SEC) +
                static_cast<uint32_t>(static_cast<uint64_t>(ts.tv_nsec) / NS_PER_AST_TOD_UNIT);

            // Calculate difference (Radar - Kernel)
            int32_t diff = static_cast<int32_t>(report.TOD) - static_cast<int32_t>(kernel_128th);

            // Handle Midnight Wrap using constexpr
            if (diff > static_cast<int32_t>(AST_TOD_HALFDAY_UNITS)) {
                diff -= static_cast<int32_t>(AST_TOD_UNITS_PER_DAY);
            } else if (diff < -static_cast<int32_t>(AST_TOD_HALFDAY_UNITS)) {
                diff += static_cast<int32_t>(AST_TOD_UNITS_PER_DAY);
            }

            // Update both reference time and the moving average offset
            sourceStateManager->updateTimeOffset(report.sourceIdentifier, diff);
        }

        // CORRECT THE REPORT for the Listeners
        // Now shift the report's TOD to match System Time
        int32_t avgOffset = sourceStateManager->getAverageOffset(report.sourceIdentifier);
        report.TOD = static_cast<uint32_t>(static_cast<int32_t>(report.TOD) - avgOffset);

        // Listener Notification
        {
            // SHARED LOCK: Multiple threads can read/notify safely
            // But blocks if someone is currently adding a listener
            std::shared_lock lock(listenerMutex);

            // Notify all valid listeners
            for (const auto& wp : listeners) {
                if (auto sp = wp.lock()) {
                    sp->onReportDecoded(report);
                }
            }
        } // Release lock here

        // Cleanup expired listeners cleanly (Need a write lock now)
        // Note: You might defer this to happen less frequently to avoid lock contention
        {
            std::unique_lock lock(listenerMutex);
            listeners.erase(
                std::remove_if(listeners.begin(), listeners.end(),
                    [](const std::weak_ptr<IAsterix2Listener>& wp) {
                        return wp.expired();
                    }),
                listeners.end()
            );
        }
    }

    return consumed;
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
