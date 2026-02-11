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

#pragma once

// System headers
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

// Own header
#include <ReactorAsterix/core/FastBitReader.h>
#include <ReactorAsterix/core/AsterixDataItemHandlerBase.h>

namespace ReactorAsterix {

/**
 * A "No-Op" item used to pad FSPEC octets
 * If this item is present in the bitstream, it means the sender
 * included a field we did not foresee in our schema.
 */
class DummyItem final : public AsterixDataItemHandlerBase {
    public:
        size_t decode([[maybe_unused]] std::string_view) override {
            return 0;
        }
};

// Static instances for padding
[[maybe_unused]] static DummyItem dummy;
[[maybe_unused]] static bool dummy_presence;

template <typename T>
    bool decode_item(FastBitReader& reader, int& bit, T& item, std::string_view& data, AsterixStatsData& stats) {
        // Read the bit from the FSPEC and store it directly in the handler's presence flag
        item.presence = reader.readBit(bit);

        // If the bit is 0, the item is not in this record; move to the next item
        if (!item.presence) {
            if (item.isMandatory()) {
                stats.protocolViolations++;
            }
            return true;
        }

        // Check if we just read a '1' for a DummyItem
        if constexpr (std::is_same_v<std::decay_t<T>, DummyItem>) {
            stats.protocolViolations++;
            return false; // STOP DECODING: Unforeseen item encountered
        }

        auto itemSize = item.decode(data);
        if (!itemSize) [[unlikely]] {
            return false;
        }

        data.remove_prefix(itemSize);
        return true;
    }

template <typename Octet>
inline bool decode_octet_inline(FastBitReader& reader, int& bit, Octet& octet,
        std::string_view& data, AsterixStatsData& stats) {
    // Manually unroll the 7 items in the octet
    return (decode_item(reader, bit, std::get<0>(octet), data, stats) &&
            decode_item(reader, bit, std::get<1>(octet), data, stats) &&
            decode_item(reader, bit, std::get<2>(octet), data, stats) &&
            decode_item(reader, bit, std::get<3>(octet), data, stats) &&
            decode_item(reader, bit, std::get<4>(octet), data, stats) &&
            decode_item(reader, bit, std::get<5>(octet), data, stats) &&
            decode_item(reader, bit, std::get<6>(octet), data, stats));
}

} // namespace ReactorAsterix


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
