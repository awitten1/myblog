#include <algorithm>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstdlib>
#include <random>
#include <vector>
#include "functions.hpp"


static void BM_BinarySearch(benchmark::State& state) {
    size_t N = state.range(0);
    auto nums = get_nums(N);
    std::sort(nums.begin(), nums.end());
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> rng{};

    for (auto _ : state) {
        long target = rng(gen);
        benchmark::DoNotOptimize(binary_search(nums, target));
    }
    state.SetComplexityN(N);
}

static void BM_LinearSearch(benchmark::State& state) {
    size_t N = state.range(0);
    auto nums = get_nums(N);
    std::sort(nums.begin(), nums.end());
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> rng{};

    for (auto _ : state) {
        long target = rng(gen);
        benchmark::DoNotOptimize(linear_search(nums, target));
    }
    state.SetComplexityN(N);
}

const long start_dense = 1 << 8;

BENCHMARK(BM_LinearSearch)->DenseRange(8,1<<12,1<<6)->Complexity();
BENCHMARK(BM_BinarySearch)->DenseRange(8,1<<12,1<<6)->Complexity();

BENCHMARK_MAIN();