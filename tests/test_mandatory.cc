#include <gtest/gtest.h>
#include <string_view>
#include <memory>
#include "ReactorAsterix/cat001/Asterix001Report.h"
#include "ReactorAsterix/cat001/Asterix001Handler.h"
#include "ReactorAsterix/core/SourceStateManager.h"

using namespace ReactorAsterix;
using namespace std::string_view_literals;

TEST(AsterixFSpecTest, DetectsMissingMandatoryItems) {
    Asterix001Report report;
    AsterixStatsData stats;

    // Setup the required parent handler and its dependency
    auto stateManager = std::make_shared<SourceStateManager>();
    Asterix001Handler handler(stateManager);

    // I001/010 (SAC/SIC) is mandatory.
    // This F-Spec (0x40) only enables the second bit (I001/020).
    auto invalidFspec = "\x40"sv;
    auto dummyData = "\x20"sv;

    // Pass the actual handler instance as the 'parent' argument
    bool result = report.process_all_octets(invalidFspec, dummyData, stats, handler);

    // Based on your technical logic, this should return false because 
    // mandatory items are missing from the F-Spec.
    EXPECT_FALSE(result);
}
