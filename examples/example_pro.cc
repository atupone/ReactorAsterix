#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <iomanip>

#include "ReactorAsterix/core/AsterixPacketHandler.h"
#include "ReactorAsterix/cat001/Asterix1Handler.h"
#include "ReactorAsterix/cat001/IAsterix1Listener.h"
#include "ReactorAsterix/core/SourceStateManager.h"

using namespace ReactorAsterix;

// Better: Encapsulate the listener logic
class TerminalLogger : public IAsterix1Listener {
public:
    void onReportDecoded(const Asterix1Report& report) override {
        std::cout << "\033[1;32m[CAT 001 Report Decoded]\033[0m\n";

        // Use SAC/SIC from report
        std::cout << "  Source: SAC=" << static_cast<int>(report.sourceIdentifier.sac) 
                  << " SIC=" << static_cast<int>(report.sourceIdentifier.sic) << "\n";

        if (report.mode3A) {
             std::cout << "  Mode 3/A: " << std::setfill('0') << std::setw(4) 
                       << std::oct << report.mode3A->code << std::dec << "\n";
        }

        std::cout << "  Position: " << report.range << "m @ " << report.azimuth << " rad\n" 
                  << std::endl;
    }
};

int main() {
    // Shared state management for tracking Time of Day (TOD)
    auto stateManager = std::make_shared<SourceStateManager>();

    // Create the handler and register it to the top-level packet handler
    AsterixPacketHandler packetHandler;

    // Use unique_ptr to manage handler ownership
    auto cat1Handler = std::make_unique<Asterix1Handler>(stateManager);

    // Register listener before moving ownership
    TerminalLogger logger;
    cat1Handler->addListener(&logger);

    packetHandler.registerCategoryHandler(1, std::move(cat1Handler));

    // Sample Packet: Category 1, Length 12, SAC/SIC, TRD, Position, Mode 3/A
    // Logic: [CAT][LEN_HI][LEN_LO][FSPEC][DATA...]
    static const std::vector<uint8_t> asterixData = {
        0x01,                   // Category
        0x00, 0x0F,             // Length (12 bytes)
        0xF8,                   // FSPEC (1111 1000) -> Items 1,2,3,4,5 present
        0x01, 0x02,             // I001/010: SAC=1, SIC=2
        0x20,                   // I001/020: Target Report Descriptor
        0x00, 0x80, 0x40, 0x00, // I001/040: Range & Azimuth
        0x00, 0x00,             // I001/070: Mode 3/A
        0x00, 0x00              // I001/090: Mode C
    };

    std::cout << "Starting ASTERIX Stream Processing...\n";
    packetHandler.handlePacket(asterixData.data(), asterixData.size());

    return 0;
}
