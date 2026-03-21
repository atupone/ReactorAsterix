#include <benchmark/benchmark.h>
#include <ReactorAsterix/cat048/Asterix048Handler.h>
#include <ReactorAsterix/core/SourceStateManager.h>
#include <string_view>
#include <memory>

using namespace ReactorAsterix;
using namespace std::string_view_literals;

class BenchListener : public IAsterix048Listener {
    public:
        void onReportDecoded(const Asterix048Report&) override {
            benchmark::DoNotOptimize(0); // Simulate minimal work
        }
};

static void BM_Cat048_WithListener(benchmark::State& state) {
    auto stateManager = std::make_shared<SourceStateManager>();
    auto handler = std::make_shared<Asterix048Handler>(stateManager);
    auto listener = std::make_shared<BenchListener>();

    handler->addListener(listener); // Register the observer

    AsterixStatsData stats;
    
    // Mock arrival time
    struct timespec ts = {1710873600, 500000000}; 

    // Raw Cat 048 Record: SAC/SIC (01/10), Message Type, and TOD (12:00:00)
    // FSPEC (0xC2), Data: \x01\x0A (SAC/SIC), \x01 (Type), \x03\xE8 (Truncated TOD)
    auto fspec = "\xC2"sv;
    auto payload = "\x01\x0A\x01\x03\xE8"sv;

    for (auto _ : state) {
        handler->processDataRecord(fspec, payload, ts, stats);
    }

    auto total_bytes = static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(payload.size());
    state.SetBytesProcessed(total_bytes);
}
BENCHMARK(BM_Cat048_WithListener);

BENCHMARK_MAIN();
