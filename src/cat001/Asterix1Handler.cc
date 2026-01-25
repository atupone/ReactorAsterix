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
    registerBatch<
        I001_010_Handler, // I001/010: Data Source Identifier
        I001_020_Handler, // I001/020: Target Report Descriptor
        I001_040_Handler, // I001/040: Measured Position in Polar Co-ordinates
        I001_070_Handler, // I001/070: Mode-3/A Code in Octal Representation
        I001_090_Handler, // I001/090: Mode-C Code in Binary Representation
        I001_130_Handler, // I001/130: Radar Plot Characteristics
        I001_141_Handler, // I001/141: Truncated Time of Day
        I001_050_Handler, // I001/050: Mode-2 Code in Octal Representation
        I001_131_Handler, // I001/131: Received Power
        I001_150_Handler  // I001/150: Presence of X-Pulse
    >();
}

void Asterix1Handler::setStats(AsterixStats& s) {
    // 1. Call the base class to set the local stats_ptr
    AsterixCategoryHandler::setStats(s);

    // 2. Propagate the reference to every handler in your compile-time tuple
    std::apply([&s](auto&&... handler) {
        (handler.setStats(s), ...);
    }, m_handlers);
}

uint32_t Asterix1Handler::calculateCurrentTod(struct timespec ts) noexcept {
    // Use the KERNEL timestamp as the fallback/reference
    // Convert timespec (seconds + nanoseconds) to ASTERIX units (1/128 sec)

    // Get the remainder of the day (0 to 86399)
    // ts.tv_sec is a 64-bit integer on modern Linux
    constexpr uint32_t SECONDS_PER_DAY = 86400;
    uint32_t secondsInDay = static_cast<uint32_t>(ts.tv_sec % SECONDS_PER_DAY);

    // Convert to 1/128s units using bit-shifts
    // secondsInDay * 128 (max value ~11 million)
    uint32_t refTime = secondsInDay << 7;

    // 3. Convert nanoseconds to 1/128s
    // We use 1e9 as the divisor.
    // Calculation: (nsec * 128) / 1,000,000,000
    // Note: (ts.tv_nsec * 128) fits comfortably in a uint64_t
    refTime += static_cast<uint32_t>((static_cast<uint64_t>(ts.tv_nsec) * 128) / 1000000000);
    return refTime;
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

bool Asterix1Handler::dispatch(int frn, Asterix1Report& report, std::string_view& data) {
    // Calls the template version in the base class
    return AsterixCategoryHandler::dispatch(frn, report, data, m_handlers);
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
        std::string_view payload,
        struct timespec ts)
{
    // Create the context object (Asterix1Report).
    Asterix1Report report;

    // Set the manager pointer so the report can resolve itself later
    // This is the "secret sauce" that allows the decoders to stay unchanged.
    report.manager = this->sourceStateManager.get();

    // Decode everything first.
    // This populates SAC/SIC and the raw 16-bit LSP Clock (if present).
    size_t consumed = processDataRecordGeneric(fspec, payload, report, m_handlers);

    if (consumed <= 0) [[unlikely]] {
        return 0;
    }

    if (!report.sourceRecord) [[unlikely]] {
        return consumed;
    }

    if (!report.sourceRecord->isSynchronized.load(std::memory_order_acquire)) [[unlikely]] {
        return consumed;
    }

    // Short-circuit: If sourceRecord has a valid lastTod, use it.
    uint32_t last = report.sourceRecord->lastTod.load(std::memory_order_relaxed);

    uint32_t ref;

    if (last > 0) [[likely]] {
        ref = last;
    } else {
        ref = calculateCurrentTod(ts);
    }

    report.TOD = report.has(Asterix1Report::Presence::HAS_LSP_CLOCK)
        ? expandTruncatedTime(report.todLSP, ref)
        : ref;

    // UPDATE MANAGER FIRST (Using raw Radar TOD)
    // ALWAYS update the Radar's 24h clock state (for bit-stitching ref)
    if (report.sourceRecord) {
        // Update persistent raw time
        report.sourceRecord->lastTod = report.TOD;

        // APPLY OFFSET FOR LISTENERS (Transition to System Domain)
        // Now we shift the report's TOD to match our local Linux clock
        // Get the average offset directly from the record (No lookup needed!)
        int32_t corrected = static_cast<int32_t>(report.TOD) -
            report.sourceRecord->averageOffset;

        // Handle the midnight wrap-around
        constexpr int32_t TICKS_PER_DAY = 86400 * 128;

        if (corrected < 0) {
            corrected += TICKS_PER_DAY; // Handles "just after midnight"
        } else if (corrected >= TICKS_PER_DAY) {
            corrected -= TICKS_PER_DAY; // Handles "just before midnight"
        }

        // Apply the shift
        report.TOD = static_cast<uint32_t>(corrected);
    }

    // LOCK-FREE NOTIFICATION
    // Just take a reference to the current list.
    // Even if a writer updates 'listeners' now, we safely iterate our local 'snapshot'.
    auto currentListeners = this->getListeners();

    // Notify all valid listeners
    for (const auto& wp : *currentListeners) {
        if (auto sp = wp.lock()) {
            sp->onReportDecoded(report);
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
