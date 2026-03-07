#include <gtest/gtest.h>
#include "ReactorAsterix/cat001/Asterix001DataItemCollection.h"

using namespace ReactorAsterix;
using namespace std::string_view_literals;

TEST(AsterixExtendedItemTest, DecodesMultiByteDescriptor) {
    I001_020_Handler handler;

    // Byte 1: 0x81 -> TYP bit set (0x80) AND FX bit set (0x01)
    // Byte 2: 0x00 -> FX bit NOT set (terminates the item)
    auto extendedData = "\x81\x00"sv;

    // Verify the handler consumed both bytes due to the FX bit.
    size_t consumed = handler.decode(extendedData);
    EXPECT_EQ(consumed, 2);

    // Verify a specific flag from the first octet.
    // According to your handler, 0x01 in the first octet would map to specific flags.
    // Replace 'extra' with the actual member mapped to that bit in your implementation.
    EXPECT_TRUE(handler.typ);
}
