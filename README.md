# ReactorAsterix

**ReactorAsterix** is a high-performance, C++17 library designed for decoding ASTERIX (All Purpose STructured Point To Point Information eXchange) surveillance data. It features a modular, listener-based architecture that simplifies the processing of complex radar data streams.

## Features

* **Multi-Category Support**: Specialized handlers for Category 001 (Target Reports) and Category 002 (Service Messages).
* **Packet Handling**: Automatic dispatching of concatenated data blocks within a single UDP/network packet.
* **State Management**: Includes a `SourceStateManager` to track Reference Time of Day (TOD) across different sensors using a map of `SourceIdentifier` to `uint32_t`.
* **Precision Decoding**: Accurate conversion of raw binary data to physical units, such as converting polar range from $1/128$ NM to meters.
* **Thread Safety**: Uses atomic counters within the `AsterixStats` structure to track performance and errors across threads.

## Project Structure

* `include/ReactorAsterix/core`: Entry points and base classes for decoding, including `AsterixPacketHandler`.
* `include/ReactorAsterix/cat001`: Category 001 (Plots) specific implementations and report structures.
* `include/ReactorAsterix/cat002`: Category 002 (North/Sector) specific implementations.
* `src/`: Implementation files for decoding logic and data item handlers.

## Getting Started

### Prerequisites
* C++17 compliant compiler (GCC 7+, Clang 5+, or MSVC 2017+).
* CMake 3.14 or higher.

### Building
```bash
mkdir build && cd build
cmake ..
make
sudo make install
```

## Usage Example

The library uses an `AsterixPacketHandler` that dispatches records to specific Category Handlers. You receive decoded data by implementing a listener interface.

```cpp
#include <ReactorAsterix/core/AsterixPacketHandler.h>
#include <ReactorAsterix/cat001/Asterix1Handler.h>

using namespace ReactorAsterix;

// 1. Create a listener for decoded reports
class MyListener : public IAsterix1Listener {
    void onReportDecoded(std::shared_ptr<Asterix1Report> report) override {
        std::cout << "Decoded Position: " << report->range << "m" << std::endl;
    }
};

int main() {
    auto state = std::make_shared<SourceStateManager>();
    AsterixPacketHandler packetHandler;

    // 2. Register Category 1 handler
    auto cat1 = std::make_unique<Asterix1Handler>(state);
    MyListener listener;
    cat1->addListener(&listener);
    
    packetHandler.registerCategoryHandler(1, std::move(cat1));

    // 3. Process raw binary data
    uint8_t buffer[] = { 0x01, 0x00, 0x09, 0x80, 0x01, 0x02, 0x00, 0x00, 0x00 };
    packetHandler.handlePacket(buffer, sizeof(buffer));

    return 0;
}
```

## Extending the Library

To add a new ASTERIX category (e.g., Cat 048), follow these steps:

1.  **Define a Report Class**: Create a class (e.g., `Asterix48Report`) to hold the decoded fields.
2.  **Implement Data Item Handlers**: Create classes for each FRN (Field Record Number) inheriting from `AsterixDataItemHandlerFixedLength` or `AsterixDataItemHandlerExtendedLength`.
3.  **Implement the Category Handler**:
    * Inherit from `IAsterixCategoryHandler`.
    * In `registerHandlers()`, map your FRN handlers using `addHandler(std::make_unique<I048_XXX_Handler>(), FRN)`.
4.  **Register with PacketHandler**: Use `packetHandler.registerCategoryHandler(48, std::move(myCat48Handler))` in your main application.

## Technical Logic: F-Spec Validation



The library ensures data integrity by validating that all required Data Items are present in the Field Specification (F-Spec). If a mandatory bit is missing—checked using the logic `mandatoryFspec[i] & ~static_cast<uint8_t>(fspec[i])`—the record is flagged as uninterpretable.

## License
Copyright (C) 2026 Alfredo Tupone. This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation.
