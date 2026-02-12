# ReactorAsterix & AtuReactor: Technical User Guide

This guide provides a comprehensive overview of the high-performance C++ framework for network packet reception and ASTERIX surveillance data decoding.

---

## 1. System Overview

The solution consists of two distinct but complementary libraries:

1.  **AtuReactor**: A low-latency networking engine that utilizes the **Reactor Pattern** for asynchronous I/O. It is "Thread-Hostile" by design to eliminate locking overhead and context switching in the hot path.
2.  **ReactorAsterix**: A decoder library for the ASTERIX (All Purpose STructured Point To Point Information eXchange) protocol. It decodes binary radar data into structured C++ reports.



### Key Capabilities
* **High Performance**: Uses `epoll` for O(1) scalability and `recvmmsg` for batch packet processing.
* **Memory Optimization**: Supports `MAP_HUGETLB` (Hugepages) to reduce TLB misses.
* **Precision Timing**: Supports kernel-level nanosecond timestamps (`SO_TIMESTAMPNS`).
* **Replay**: Native support for replaying `.pcap` files for testing and simulation.
* **ASTERIX Categories**: Out-of-the-box support for Categories 001, 002, 034, and 048.

---

## 2. Architecture & Data Flow

The system processes data in a pipeline: **Network -> Packet Handler -> Category Handler -> Listener**.

### A. The Network Layer (`AtuReactor`)
The core is the `EventLoop`, which manages file descriptors. You use a `UDPReceiver` or `PcapReceiver` to feed data into the loop.
* **UDPReceiver**: Binds to a port (IPv4/IPv6 dual-stack) and uses a pre-allocated ring buffer to read packets.
* **PcapReceiver**: Maps a capture file into memory and replays it, respecting the original packet timestamps.

### B. The Decoding Layer (`ReactorAsterix`)
The `AsterixPacketHandler` sits between the network and the application logic.
1.  **Ingest**: It receives a raw binary block.
2.  **Identify**: It parses the Header (Category + Length).
3.  **Dispatch**: It looks up the registered `IAsterixCategoryHandler` for that Category ID.
4.  **Decode**: The specific handler parses the Field Specification (FSPEC) and Data Items.
5.  **Notify**: The decoded report is sent to all registered `Listeners`.



---

## 3. Quick Start Guide

### Step 1: Building the Libraries
Both projects use CMake.

```bash
# Build AtuReactor
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Build ReactorAsterix
mkdir build && cd build
cmake ..
make
```

### Step 2: Creating a Listener
Define a class that inherits from the specific Listener interface (e.g., `IAsterix001Listener`).

```cpp
#include <ReactorAsterix/cat001/IAsterix001Listener.h>
#include <iostream>

using namespace ReactorAsterix;

class MyRadarListener : public IAsterix001Listener {
public:
    void onReportDecoded(const Asterix001Report& report) override {
        std::cout << "Target Range: " << report.i001_040.range << " meters" << std::endl;
        std::cout << "Azimuth: " << report.i001_040.azimuth << std::endl;
    }
};
```

### Step 3: Wiring it Together (Network + Decoder)
This example binds a UDP listener on port 4321 and feeds data into the ASTERIX decoder.

```cpp
#include <atu_reactor/EventLoop.h>
#include <atu_reactor/UDPReceiver.h>
#include <ReactorAsterix/core/AsterixPacketHandler.h>
#include <ReactorAsterix/cat001/Asterix001Handler.h>

// Bridge function to link AtuReactor callback to AsterixPacketHandler
void bridge(void* ctx, const uint8_t* data, size_t len, uint32_t, struct timespec ts) {
    static_cast<ReactorAsterix::AsterixPacketHandler*>(ctx)->handlePacket(data, len, ts);
}

int main() {
    // 1. Setup ASTERIX State & Handlers
    auto state = std::make_shared<ReactorAsterix::SourceStateManager>();
    ReactorAsterix::AsterixPacketHandler packetHandler;
    
    auto cat1 = std::make_unique<ReactorAsterix::Asterix001Handler>(state);
    cat1->addListener(std::make_shared<MyRadarListener>()); 
    packetHandler.registerCategoryHandler(1, std::move(cat1));

    // 2. Setup Network Reactor
    atu_reactor::EventLoop loop;
    atu_reactor::UDPReceiver receiver(loop);

    // 3. Subscribe to Port 4321
    receiver.subscribe(4321, &packetHandler, bridge);

    // 4. Run Loop
    while (true) { loop.runOnce(1000).value(); }
    return 0;
}
```

---

## 4. Advanced Configuration

### Hugepages (Performance Tuning)
To maximize throughput and reduce CPU cache misses, `AtuReactor` attempts to use Hugepages (2MB pages) for packet buffers.
* **Requirement**: You must reserve hugepages in the OS: `sudo sysctl -w vm.nr_hugepages=512`.
* **Fallback**: If hugepages are unavailable, it automatically falls back to standard 4KB pages.

### Time Synchronization
Surveillance data relies on accurate timestamps. `ReactorAsterix` includes a `SourceStateManager` to handle the transition from Radar Time (cyclic 24h clock) to System Time (Unix Epoch). It calculates the average offset between the Radar TOD and the Kernel receipt time (`SO_TIMESTAMPNS`) to maintain synchronization even if the radar clock drifts.

### PCAP Replay Modes
The `PcapReceiver` supports three replay modes:
1.  **TIMED**: Respects the original packet timing relative to the wall clock.
2.  **FLOOD**: Replays as fast as the CPU allows.
3.  **STEP**: Waits for a manual trigger to process the next packet.

---

## 5. Extending the Library

To add support for a new ASTERIX Category:

1.  **Define Reports**: Create a report class inheriting from `AsterixReport`.
2.  **Create Item Handlers**: Create classes for every Data Item (FRN) inheriting from `AsterixDataItemHandlerFixedLength` or `ExtendedLength`.
3.  **Implement Category Handler**: Inherit from `AsterixCategoryHandler` and register your item handlers in the correct order corresponding to the FSPEC bits.
4.  **Register**: In your `main()`, register the new handler to the `AsterixPacketHandler`.

---

## 6. Troubleshooting & Statistics

The `AsterixPacketHandler` maintains atomic statistics for debugging:

| Statistic | Description |
| :--- | :--- |
| `malformedBlocks` | Header indicates a size larger than the buffer. |
| `unhandledCategories` | Valid block, but no handler registered for that ID. |
| `protocolViolations` | Mandatory item missing from FSPEC. |
| `trailingBytesCount` | Bytes left over at the end of a UDP packet. |
