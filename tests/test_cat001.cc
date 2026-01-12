#include <gtest/gtest.h>
#include "ReactorAsterix/cat001/Asterix1DataItemCollection.h"
#include "ReactorAsterix/cat001/Asterix1Report.h"

using namespace ReactorAsterix;

TEST(Asterix1HandlerTest, DecodePolarCoordinates) {
    Asterix1Report report;
    I001_040_Handler handler;

    // 4 bytes: 2 for range, 2 for azimuth
    // Range: 0x0080 (128) -> 128/128 * 1852.0 = 1852.0 meters
    // Azimuth: 0x4000 (16384) -> 16384 * (PI/32768) = PI/2 radians
    std::string data("\x00\x80\x40\x00", 4);

    handler.decode(report, data);

    EXPECT_NEAR(report.range, 1852.0, 0.1);
    EXPECT_NEAR(report.azimuth, 1.570796, 0.0001);
}
