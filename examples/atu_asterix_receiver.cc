#include <iostream>
#include <memory>

#include <atu_reactor/EventLoop.h>
#include <atu_reactor/UDPReceiver.h>
#include <atu_reactor/Types.h>

#include "ReactorAsterix/core/AsterixPacketHandler.h"
#include "ReactorAsterix/cat001/Asterix1Handler.h"
#include "ReactorAsterix/cat001/IAsterix1Listener.h"
#include "ReactorAsterix/core/SourceStateManager.h"

using namespace ReactorAsterix;
using namespace atu_reactor;

// A simple printer for decoded radar reports
class AsterixPrinter : public IAsterix1Listener {
public:
    void onReportDecoded(const Asterix1Report& report) override {
        std::cout << "[CAT001] Decoded - Range: " << report.range << "m" << std::endl;
    }
};

/**
 * Static bridge function to convert the raw C-style callback into
 * a call to our C++ AsterixPacketHandler object.
 */
void asterix_callback_bridge(void* context, const uint8_t* data, size_t size, uint32_t flags) {
    (void)flags; // Reserved for future use by AtuReactor
    if (context && data && size > 0) {
        auto* handler = static_cast<AsterixPacketHandler*>(context);
        handler->handlePacket(data, size);
    }
}

int main() {
    // 1. Setup Asterix logic
    auto stateManager = std::make_shared<SourceStateManager>();
    AsterixPacketHandler packetHandler;
    
    auto cat1 = std::make_unique<Asterix1Handler>(stateManager);
    auto printer = std::make_shared<AsterixPrinter>();
    cat1->addListener(printer);
    packetHandler.registerCategoryHandler(1, std::move(cat1));

    // 2. Setup Network Reactor
    EventLoop loop;
    UDPReceiver receiver(loop); 

    // 3. Subscribe using the static bridge and passing &packetHandler as context
    uint16_t port = 4321;
    auto result = receiver.subscribe(port, &packetHandler, asterix_callback_bridge);

    // Result uses .error() based on your compiler hint
    if (result.error()) {
        std::cerr << "Failed to subscribe to port " << port << std::endl;
        return 1;
    }

    std::cout << "ASTERIX Receiver active on UDP port " << port << "..." << std::endl;
    
    // 4. Drive the loop
    while (true) {
        loop.runOnce(1000); // Process events with 1s timeout 
    }

    return 0;
}
