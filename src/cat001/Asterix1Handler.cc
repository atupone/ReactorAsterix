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
#include <ReactorAsterix/cat001/Asterix1Handler.h>

// System headers
#include <cmath>
#include <chrono>

// Library headers
#include <ReactorAsterix/cat001/Asterix1DataItemCollection.h>

namespace ReactorAsterix {

/**
 * @brief Constructor for the ASTERIX Category 1 Handler.
 *
 * Calls the virtual method to register all specific data item handlers.
 */
Asterix1Handler::Asterix1Handler(std::shared_ptr<SourceStateManager> manager)
    : sourceStateManager(manager) {
    registerHandlers();
}

/**
 * @brief Registers the specific handlers for ASTERIX Category 1 data items.
 *
 * This method overrides the pure virtual method from the base class. It populates
 * the `handlers` vector with unique pointers to the concrete handler classes,
 * mapping each handler to its corresponding Field Record Number (FRN).
 */
void Asterix1Handler::registerHandlers() {
    // Register handlers at index = FRN - 1.
    addHandler(std::make_unique<I001_010_Handler>(),  1); // I001/010: Data Source Identifier
    addHandler(std::make_unique<I001_020_Handler>(),  2); // I001/020: Target Report Descriptor
    addHandler(std::make_unique<I001_040_Handler>(),  3); // I001/040: Measured Position in Polar Co-ordinates
    addHandler(std::make_unique<I001_070_Handler>(),  4); // I001/070: Mode-3/A Code in Octal Representation
    addHandler(std::make_unique<I001_090_Handler>(),  5); // I001/090: Mode-C Code in Binary Representation
    addHandler(std::make_unique<I001_130_Handler>(),  6); // I001/130: Radar Plot Characteristics
    addHandler(std::make_unique<I001_141_Handler>(),  7); // I001/141: Truncated Time of Day
    addHandler(std::make_unique<I001_050_Handler>(),  8); // I001/050: Mode-2 Code in Octal Representation
    addHandler(std::make_unique<I001_131_Handler>(), 10); // I001/131: Received Power
    addHandler(std::make_unique<I001_150_Handler>(), 15); // I001/150: Presence of X-Pulse
}

uint32_t Asterix1Handler::calculateCurrentTod() noexcept {
    using namespace std::chrono;

    // Get time since epoch
    auto now = system_clock::now().time_since_epoch();

    // Use floor to get the number of whole days (24h periods) since epoch
    auto days = floor<duration<int, std::ratio<86400>>>(now);

    // Get the duration since the start of the current day
    auto since_midnight = now - days;

    // ASTERIX TOD: 1 second = 128 units
    // We convert the duration since midnight to microseconds first for precision,
    // then scale to ASTERIX units (1/128s).
    auto micros = duration_cast<microseconds>(since_midnight).count();

    // 128 / 1,000,000 = 1 / 7812.5
    // Better to multiply by 128 then divide by 1,000,000 to maintain precision
    return static_cast<uint32_t>((static_cast<uint64_t>(micros) * 128) / 1000000);
}

uint32_t Asterix1Handler::expandTruncatedTime(uint16_t todLSP, uint32_t refTOD) noexcept {
    constexpr uint32_t maxTOD   = 86400 * 128;
    constexpr uint32_t kMspMask = 0xFFFF0000;
    constexpr uint32_t kWindow  = 0x00010000;

    const uint32_t kTopMsp  = (maxTOD - 1) & kMspMask;
    const uint32_t HALF_DAY = maxTOD / 2;

    const uint32_t refMSP = refTOD & kMspMask;
    const uint32_t lsp    = static_cast<uint32_t>(todLSP);

    const uint32_t todA = refMSP | lsp;

    // Calculate candidate B (Crossing lower boundary)
    const uint32_t todB = (refMSP > 0)       ? (todA - kWindow) : (kTopMsp | lsp);

    // Calculate candidate C (Crossing upper boundary)
    const uint32_t todC = (refMSP < kTopMsp) ? (todA + kWindow) : lsp;

    auto getDist = [refTOD, maxTOD, HALF_DAY](uint32_t T) -> uint32_t {
        if (T >= maxTOD) return maxTOD;
        uint32_t d = (T > refTOD) ? (T - refTOD) : (refTOD - T);
        return (d > HALF_DAY) ? (maxTOD - d) : d;
    };

    uint32_t bestT   = todA;
    uint32_t minDist = getDist(todA);

    // Check Candidate B
    if (uint32_t dB = getDist(todB); dB < minDist) {
        minDist = dB;
        bestT   = todB;
    }

    // Check Candidate C
    if (uint32_t dC = getDist(todC); dC < minDist) {
        bestT   = todC;
    }

    return bestT;
}

/**
 * @brief Handles the processing of a single ASTERIX Category 1 data record (Plot).
 *
 * This function is the entry point for processing a plot message. It initializes
 * an 'Asterix1Report` context object, parses the data using the F-spec logic
 * from the base class, and then conditionally passes the completed plot to the
 * tracking system.
 *
 * @param fspec A pointer to the start of the Field Specification.
 * @param fspecSize The size of the F-spec in bytes.
 * @param data A pointer to the start of the data payload.
 * @param dataLeft The remaining size of the data payload in bytes.
 * @return size_t The total number of bytes consumed by this handler.
 */
size_t Asterix1Handler::processDataRecord(
        std::string_view fspec,
        std::string_view payload)
{
    // Create the context object (Asterix1Report).
    Asterix1Report report;

    // Decode everything first.
    // This populates SAC/SIC and the raw 16-bit LSP Clock (if present).
    size_t consumed = this->_processDataRecordInternal(fspec, payload, report);

    if (consumed > 0) {
        // Get the best available 24-bit reference time
        uint32_t ref = sourceStateManager->getReferenceTime(
                report.sourceIdentifier).value_or(calculateCurrentTod());

        report.TOD = report.hasLspClock
            ? expandTruncatedTime(report.todLSP, ref)
            : ref;

        // Update state with the radar's actual 32-bit time for the next message
        sourceStateManager->updateSourceTime(report.sourceIdentifier, report.TOD);

        // SHARED LOCK: Multiple threads can read/notify safely
        // But blocks if someone is currently adding a listener
        std::shared_lock lock(listenerMutex);

        // Notify all registered listeners

        // Use an iterator-based loop so we can remove dead listeners
        auto it = listeners.begin();
        while (it != listeners.end()) {
            // Attempt to get a temporary shared_ptr
            if (auto strongListener = it->lock()) {
                strongListener->onReportDecoded(report);
                ++it;
            } else {
                // The listener was deleted elsewhere!
                // This is safe cleanup.
                it = listeners.erase(it);
            }
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
