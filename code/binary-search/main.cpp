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

const long end_dense_first = 1 << 8;
const long end_dense = 1 << 11;
const long end_range = 1 << 18;

BENCHMARK(BM_LinearSearch)->DenseRange(8,end_dense_first,1<<5)
    ->DenseRange(end_dense_first,end_dense,1<<7)
    ->RangeMultiplier(2)->Range(end_dense, end_range)
    ->Complexity();

BENCHMARK(BM_BinarySearch)->DenseRange(8,end_dense_first,1<<5)
    ->DenseRange(end_dense_first,end_dense,1<<7)
    ->RangeMultiplier(2)->Range(end_dense, end_range)
    ->Complexity();

BENCHMARK_MAIN();