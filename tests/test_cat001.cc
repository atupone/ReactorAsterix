#include <gtest/gtest.h>
#include "ReactorAsterix/cat001/Asterix001DataItemCollection.h"
#include "ReactorAsterix/cat001/Asterix001Report.h"

using namespace ReactorAsterix;

TEST(Asterix1HandlerTest, DecodePolarCoordinates) {
    Asterix001Report report;
    I001_040_Handler handler;

    // 4 bytes: 2 for range, 2 for azimuth
    // Range: 0x0080 (128) -> 128/128 * 1852.0 = 1852.0 meters
    // Azimuth: 0x4000 (16384) -> 16384 * (PI/32768) = PI/2 radians
    std::string data("\x00\x80\x40\x00", 4);

    handler.decode(data);

    EXPECT_NEAR(handler.range, 1, 0.1);
    EXPECT_NEAR(handler.azimuth, 1, 0.1);
}
