#include <gtest/gtest.h>
#include <memory>
#include <string_view>
#include "ReactorAsterix/cat001/Asterix001Handler.h"
#include "ReactorAsterix/core/SourceStateManager.h"

using namespace ReactorAsterix;
using namespace std::string_view_literals;

class MockListener : public IAsterix001Listener {
public:
    uint32_t lastTod = 0;
    bool lastSync = false;
    void onReportDecoded(const Asterix001Report& report) override {
        lastTod = report.TOD;
        lastSync = report.timeSynchronized;
    }
};

TEST(Asterix001HandlerTest, FullProcessDataFlow) {
    auto stateManager = std::make_shared<SourceStateManager>();
    Asterix001Handler handler(stateManager);
    auto listener = std::make_shared<MockListener>();
    handler.addListener(listener);

    AsterixStatsData stats;
    struct timespec ts = {43200, 0}; // 12:00:00 UTC

    // FSPEC: 0xC2 (Items 010, 020, and 141 present)
    auto fspec = "\xC2"sv;

    // Data: [SAC/SIC] + [Descriptor] + [Truncated TOD 1000]
    auto data = "\x01\x0A\x00\x03\xE8"sv;

    // PHASE 1: Cold Start.
    // lastTod expands relative to 'ts' (System Time).
    handler.processDataRecord(fspec, data, ts, stats);

    ASSERT_NE(listener->lastTod, 0);
    EXPECT_FALSE(listener->lastSync);

    // Capture the anchor created by the cold start
    uint32_t firstReportTod = listener->lastTod;

    // PHASE 2: North Sync
    SourceIdentifier sid{1, 10};
    stateManager->updateTimeOffset(sid, 100); // Radar is 100 ticks fast

    // PHASE 3: Synchronized expansion
    // Truncated TOD 1256 (0x04E8) -> 256 ticks after the first report
    auto data2 = "\x01\x0A\x00\x04\xE8"sv;
    handler.processDataRecord(fspec, data2, ts, stats);

    EXPECT_TRUE(listener->lastSync);
    // Result should be: (FirstReport + 256) - 100
    EXPECT_EQ(listener->lastTod, (firstReportTod + 256) - 100);
}
