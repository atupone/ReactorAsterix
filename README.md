# ReactorAsterix

**ReactorAsterix** is a high-performance, C++17 library designed for decoding ASTERIX (All Purpose STructured Point To Point Information eXchange) surveillance data. It features a modular, listener-based architecture that simplifies the processing of complex radar data streams.

## 🚀 Performance Benchmarks
*Tested on x86_64 @ 5.1 GHz*

| Operation | Latency | Throughput | Key Optimization |
| :--- | :--- | :--- | :--- |
| **Time Anchoring** | **0.19 ns** | ~5.2 Billion ops/s | Register-passing ABI (Zero memory load) |
| **Full Cat 048 Decode** | **11.1 ns** | **89.9 Million** msg/s | Zero-allocation hot path |
| **With Observer/Listener** | **12.0 ns** | **83.3 Million** msg/s | COW Lock-free notifications |

## Features

* **Multi-Category Support**: Specialized handlers for Category 001/002 & 034/048.
* **Lock-Free Notifications**: Uses a Copy-On-Write (COW) mechanism with atomic pointers to notify listeners without blocking the decoding pipeline.
* **Zero-Allocation Hot Path**: Category handlers use persistent member variables and high-speed local caches to eliminate heap allocation and TLS lookups.
* **Packet Handling**: Automatic dispatching of concatenated data blocks within a single UDP/network packet.
* **State Management**: Includes a `SourceStateManager` to track Reference Time of Day (TOD) across different sensors using a map of `SourceIdentifier` to `uint32_t`.
* **Precision Decoding**: Accurate conversion of raw binary data to physical units, such as converting polar range from $1/128$ NM to meters.
* **Thread Safety**: Uses atomic counters within the `AsterixStats` structure to track performance and errors across threads.

## Project Structure

* `include/ReactorAsterix/core`: Entry points and base classes for decoding, including `AsterixPacketHandler`.
* `include/ReactorAsterix/cat001/002/034/048`: Category-specific implementations and report structures.
* `src/`: Implementation files for decoding logic and data item handlers.

## Getting Started

### Prerequisites
* C++17 compliant compiler (GCC 7+, Clang 5+, or MSVC 2017+).
* CMake 3.14 or higher.
* **Google Benchmark** (Optional, for performance validation).

### Building
```bash
mkdir build && cd build
cmake .. -DBUILD_TESTING=ON
make
./unit_tests
./bench_cat048_decode
sudo make install
```

## Usage Example

The library uses an `AsterixPacketHandler` that dispatches records to specific Category Handlers. You receive decoded data by implementing a listener interface.

```cpp
#include <ReactorAsterix/core/AsterixPacketHandler.h>
#include <ReactorAsterix/cat048/Asterix048Handler.h>

using namespace ReactorAsterix;

// 1. Create a listener for decoded reports
class MyListener : public IAsterixi048Listener {
    void onReportDecoded(const Asterix048Report& report) override {
        // Access high-precision AbsoluteTime (std::chrono::time_point)
        auto arrival = report.TOD;
    }
};

int main() {
    auto state = std::make_shared<SourceStateManager>();
    AsterixPacketHandler packetHandler;

    // 2. Register Category 048 handler
    auto cat48 = std::make_unique<Asterix048Handler>(state);

    auto listener = std::make_shared<MyListener>();
    cat48->addListener(listener);
    
    packetHandler.registerCategoryHandler(48, std::move(cat48));

    // Process with arrival timestamp
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    // 3. Process raw binary data
    uint8_t buffer[] = { 0x01, 0x00, 0x09, 0x80, 0x01, 0x02, 0x00, 0x00, 0x00 };

    // handlePacket now requires a timestamp for TOD synchronization
    packetHandler.handlePacket(buffer, sizeof(buffer), ts);

    return 0;
}
```

## Extending the Library

To add a new ASTERIX category (e.g., Cat 048), follow these steps:

1.  **Define a Report Class**: Create a class (e.g., `Asterix48Report`) to hold the decoded fields.
2.  **Implement Data Item Handlers**: Create classes for each FRN (Field Record Number) inheriting from `AsterixDataItemHandlerFixedLength` or `AsterixDataItemHandlerExtendedLength`.
3.  **Implement the Category Handler**:
    * Inherit from `IAsterixCategoryHandler<Report, Listener, Handler>`.
    * Define a `HandlerTypes` tuple containing all your data item handlers
    * In `registerHandlers()`, use `registerBatch<HandlerTypes...>()` to automatically register all items based on their static `FRN` member.
4.  **Register with PacketHandler**: Use `packetHandler.registerCategoryHandler(48, std::move(myCat48Handler))` in your main application.

## Technical Logic: F-Spec Validation

The library ensures data integrity by validating that all required Data Items are present in the Field Specification (F-Spec). If a mandatory bit is missing—checked using the logic `mandatoryFspec[i] & ~static_cast<uint8_t>(fspec[i])`—the record is flagged as uninterpretable.

## License
Copyright (C) 2026 Alfredo Tupone. This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation.
