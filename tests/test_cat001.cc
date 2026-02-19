#include <gtest/gtest.h>
#include "ReactorAsterix/cat001/Asterix001DataItemCollection.h"

using namespace ReactorAsterix;
using namespace std::string_view_literals;

TEST(Asterix1HandlerTest, DecodePolarCoordinates) {
    I001_040_Handler handler;

    // C++17 string_view literal: No allocation overhead, size automatically deduced
    auto data = "\x00\x80\x40\x00"sv;

    // Verify the handler successfully consumed exactly 4 bytes
    EXPECT_EQ(handler.decode(data), 4);

    // 4 bytes: 2 for range, 2 for azimuth
    // Range: 0x0080 (128) -> 128/128 * 1852.0 = 1852.0 meters
    // Azimuth: 0x4000 (16384) -> 16384 * (PI/32768) = PI/2 radians

    EXPECT_EQ(handler.range, 128);
    EXPECT_EQ(handler.azimuth, 16384);
}
