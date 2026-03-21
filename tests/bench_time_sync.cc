#include <benchmark/benchmark.h>
#include <ReactorAsterix/core/AsterixTime.h>
#include <ReactorAsterix/cat001/Asterix001Handler.h> // If you want to bench the whole handler
#include <ctime>

using namespace ReactorAsterix;

// Benchmark the raw anchoring utility
static void BM_AsterixTime_Anchor(benchmark::State& state) {
    uint32_t mock_ticks = 43200 * 128; // 12:00:00
    struct timespec ts = {43201, 500000000}; // 12:00:01.5

    for (auto _ : state) {
        auto result = AsterixTime::anchor(mock_ticks, ts);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AsterixTime_Anchor);

BENCHMARK_MAIN();
