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
#include <ReactorAsterix/cat001/Asterix001Handler.h>

// Library headers
#include <ReactorAsterix/core/AsterixTime.h>

namespace ReactorAsterix {

Asterix001Handler::Asterix001Handler(std::shared_ptr<SourceStateManager> manager)
    : AsterixCategoryHandler(std::move(manager)) {}

uint32_t Asterix001Handler::calculateCurrentTod(struct timespec ts) noexcept {
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

uint32_t Asterix001Handler::expandTruncatedTime(uint16_t todLSP, uint32_t refTOD) noexcept {
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

void Asterix001Handler::onAfterDecode(struct timespec ts)
{
    // Sync Status Check
    // Set by your North message logic to decide if the TOD quality is 'high-precision'.
    report.timeSynchronized = report.sourceRecord->isSynchronized.load(std::memory_order_acquire);

    // Anchor Selection for 16-bit Expansion
    // The 'ref' provides the 24-hour context needed to expand truncated 16-bit data.
    uint32_t ref;

    // Load the historical anchor from the source record.
    int32_t last = report.sourceRecord->lastTod.load(std::memory_order_relaxed);

    if (last < 0) {
        // SENTINEL CASE: First packet for this source.
        // We use the arrival time (rcTime) as the first 24h reference point.
        ref = calculateCurrentTod(ts);
    } else {
        // CONTINUITY CASE: Use the previously stored radar time as the anchor.
        ref = static_cast<uint32_t>(last);
    }

    // Mathematical Expansion
    // Turn the 16-bit LSPs into a full 24-hour Time of Day.
    uint32_t TOD = report.i001_141.presence
        ? expandTruncatedTime(report.i001_141.todLSP, ref)
        : ref;

    // Reporting & State Persistence
    if (report.timeSynchronized) [[likely]] {
        uint32_t correctedTicks = applyTimeCorrection(TOD, *report.sourceRecord);
        // POST-NORTH: Use the stable offset to shift radar time into system domain.
        // This call also internally updates report.sourceRecord->lastTod.
        report.TOD = AsterixTime::anchor(correctedTicks, ts);
    } else {
        // PRE-NORTH: Report arrival-based TOD ("The message is there").
        report.TOD = AsterixTime::anchor(TOD, ts);

        // Manually update the anchor so the next packet expands relative to this one.
        report.sourceRecord->lastTod.store(static_cast<int32_t>(TOD), std::memory_order_relaxed);
    }
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
