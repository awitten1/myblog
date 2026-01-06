#include <algorithm>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>
#include "functions.hpp"

constexpr static uint64_t target_vec_size = 1 << 20;
static std::vector<long> targets(target_vec_size);
static bool initialized = false;

static void initialize_targets(std::vector<long>& target,
    long low = std::numeric_limits<long>::min(), long high = std::numeric_limits<long>::max()) {
    if (initialized) {
        return;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long> rng{low, high};
    for (auto& t : target) {
        t = rng(gen);
    }
    initialized = true;
}


template<bool randomize_search>
static void BM_BinarySearch(benchmark::State& state,
    long low = std::numeric_limits<long>::min(), long high = std::numeric_limits<long>::max()) {
    size_t N = state.range(0);
    auto nums = get_nums(N);
    std::sort(nums.begin(), nums.end());

    initialize_targets(targets, low, high);

    uint64_t i = 0;
    for (auto _ : state) {
        long target = targets.back();
        if constexpr (randomize_search) {
            target = targets[i++ & (target_vec_size - 1)];
        }
        benchmark::DoNotOptimize(binary_search(nums, target));
    }
    state.SetComplexityN(N);
}

static void BM_BinarySearchRandomTarget(benchmark::State& state) {
    BM_BinarySearch<true>(state);
}

static void BM_BinarySearchPredictableTarget(benchmark::State& state) {
    BM_BinarySearch<true>(state, 10,15);
}


static void BM_LinearSearch(benchmark::State& state) {
    size_t N = state.range(0);
    auto nums = get_nums(N);
    std::sort(nums.begin(), nums.end());
    initialize_targets(targets);

    long i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(linear_search(nums, targets[i++ & (target_vec_size - 1)]));
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

BENCHMARK(BM_BinarySearchRandomTarget)->DenseRange(8,end_dense_first,1<<5)
    ->DenseRange(end_dense_first,end_dense,1<<7)
    ->RangeMultiplier(2)->Range(end_dense, end_range)
    ->Complexity();


BENCHMARK(BM_BinarySearchPredictableTarget)->DenseRange(8,end_dense_first,1<<5)
    ->DenseRange(end_dense_first,end_dense,1<<7)
    ->RangeMultiplier(2)->Range(end_dense, end_range)
    ->Complexity();

BENCHMARK_MAIN();