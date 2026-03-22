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

// System headers
#include <iostream>
#include <memory>
#include <unistd.h>

#include <atu_reactor/EventLoop.h>
#include <atu_reactor/PcapReceiver.h>
#include <ReactorAsterix/core/AsterixPacketHandler.h>
#include <ReactorAsterix/core/SourceStateManager.h>

// Category Handlers
#include <ReactorAsterix/cat001/Asterix001Handler.h>
#include <ReactorAsterix/cat002/Asterix002Handler.h>
#include <ReactorAsterix/cat034/Asterix034Handler.h>
#include <ReactorAsterix/cat048/Asterix048Handler.h>

using namespace atu_reactor;
using namespace ReactorAsterix;

// Bridge callback that takes packets from AtuReactor and gives them to ReactorAsterix
void onPacketReceived(void* context, const uint8_t* data, size_t size, uint32_t /*status*/, struct timespec ts) {
    auto* handler = static_cast<AsterixPacketHandler*>(context);
    // ReactorAsterix handles the internal records within the packet
    handler->handlePacket(data, size, ts);
}

int main(int argc, char** argv) {
    ReplayMode mode = ReplayMode::TIMED; // Default to timed replay
    int iterations = 1;
    uint16_t port = 5001; // Default ASTERIX port
    int opt;

    // Correct flag logic from pcap_replay.cc
    while ((opt = getopt(argc, argv, "fn:p:")) != -1) {
        switch (opt) {
            case 'f':
                mode = ReplayMode::FLOOD; // Replay as fast as possible
                break;
            case 'n':
                iterations = std::atoi(optarg);
                break;
            case 'p':
                port = static_cast<uint16_t>(std::atoi(optarg));
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-f] [-n iterations] <file.pcap>\n";
                return 1;
        }
    }

    if (optind >= argc) {
        std::cerr << "Expected PCAP file after options\n";
        return 1;
    }

    char* pcapFile = argv[optind];

    EventLoop loop;
    PcapConfig config;
    config.mode = mode;

    PcapReceiver player(loop, config);
    if (auto res = player.open(pcapFile); !res) {
        std::cerr << "Failed to open PCAP: " << res.error().message() << std::endl;
        return 1;
    }

    // 1. Setup Asterix logic
    auto state = std::make_shared<SourceStateManager>();
    AsterixPacketHandler asterixHandler;

    // 2. Register all Category Handlers
    // Category 001: Target Reports
    asterixHandler.registerCategoryHandler(std::make_unique<Asterix001Handler>(state));

    // Category 002: Service Messages
    asterixHandler.registerCategoryHandler(std::make_unique<Asterix002Handler>(state));

    // Category 034: Monoradar Service Messages
    asterixHandler.registerCategoryHandler(std::make_unique<Asterix034Handler>(state));

    // Category 048: Monoradar Target Reports
    asterixHandler.registerCategoryHandler(std::make_unique<Asterix048Handler>(state));

    // 3. Subscribe the Asterix handler to the PCAP stream
    // Assuming ASTERIX traffic is on port 5001
    auto subResult = player.subscribe(port, &asterixHandler, onPacketReceived);
    if (!subResult) {
        std::cerr << "Failed to subscribe: " << subResult.error().message() << std::endl;
        return 1;
    }

    std::cout << "Starting ASTERIX Replay from: " << pcapFile << std::endl;

    for (int i = 0; i < iterations; ++i) {
        player.rewind();
        player.start();
        while (!player.isFinished()) {
            loop.runOnce(100); // Poll every 100ms
        }
    }

    std::cout << "Replay finished." << std::endl;

    asterixHandler.forceFlush();

    auto stats = asterixHandler.getStats().snapshot();
    std::cout << "Processed packets: " << stats.totalPackets << std::endl;
    std::cout << "trailingBytesCount: " << stats.trailingBytesCount << std::endl;
    std::cout << "unhandledCategories: " << stats.unhandledCategories << std::endl;
    std::cout << "malformedBlocks: " << stats.malformedBlocks << std::endl;
    std::cout << "malformedRecords: " << stats.malformedRecords << std::endl;
    std::cout << "recordParseErrors: " << stats.recordParseErrors << std::endl;
    std::cout << "protocolViolations: " << stats.protocolViolations << std::endl;
    std::cout << "unhandledItems: " << stats.unhandledItems << std::endl;
    std::cout << "uninterpretedItems: " << stats.uninterpretedItems << std::endl;

    return 0;
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 4 ***
// c-basic-offset: 4 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=4 tabstop=4
