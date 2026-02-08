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
#include <tuple>
#include <string_view>
#include <utility>

// Own header
#include <ReactorAsterix/core/FastBitReader.h>
#include <ReactorAsterix/core/AsterixDataItemHandlerBase.h>

namespace ReactorAsterix {

/**
 * A "No-Op" item used to pad FSPEC octets
 */
class DummyItem : AsterixDataItemHandlerBase {
    size_t getSize(std::string_view) const override {
        return 0;
    }
};

// Static instances for padding
[[maybe_unused]] static DummyItem dummy;
[[maybe_unused]] static bool dummy_presence;

template <typename T>
    bool decode_item(FastBitReader& reader, int& bit, T& item, std::string_view& data) {
        // Read the bit from the FSPEC and store it directly in the handler's presence flag
        item.presence = reader.readBit(bit);

        // If the bit is 0, the item is not in this record; move to the next item
        if (!item.presence) return true;

        auto itemSize = item.getSize(data);
        if (itemSize > data.size()) [[unlikely]] {
            return false;
        }

        item.decode(data.substr(0, itemSize));
        data.remove_prefix(itemSize);
        return true;
    }

// Recursive processor that walks through the "Octet" tuples
template <size_t I = 0, typename... Octets>
    bool decode_fspec_recursive(
            FastBitReader& reader,
            int& bit,
            std::string_view& data,
            std::tuple<Octets...>& schema)
    {
        if constexpr (I >= sizeof...(Octets)) {
            return true;
        } else {
            const auto& current_octet = std::get<I>(schema);
            constexpr size_t num_fields = std::tuple_size_v<std::decay_t<decltype(current_octet)>>;

            static_assert(num_fields <= 7, "An ASTERIX FSPEC octet cannot have more than 7 data fields.");

            // Decode available fields
            bool success = std::apply([&](auto&... items) {
                    return (decode_item(reader, bit, items, data) && ...);
                    }, current_octet);

            if (!success) return false;

            // If the tuple has < 7 items, skip the remaining bits to reach the FX bit position
            if constexpr (num_fields < 7) {
                reader.skipBits(bit, static_cast<int>(7 - num_fields));
            }

            // Read the 8th bit (FX - Field Extension)
            bool has_extension = reader.readBit(bit);

            if (has_extension) {
                if constexpr (I + 1 < sizeof...(Octets)) {
                    return decode_fspec_recursive<I + 1>(reader, bit, data, schema);
                }
            }

            return true;
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
