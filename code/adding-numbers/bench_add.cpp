#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>
#include <immintrin.h>

static std::vector<int64_t> make_data(size_t n) {
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int64_t> dist;
    std::vector<int64_t> v(n);
    for (auto& x : v) x = dist(rng);
    return v;
}

__attribute__((noinline))
int64_t add_integers(const int64_t* nums, size_t sz) {
    int64_t ret = 0;
    for (size_t i = 0; i < sz; ++i) {
        ret += nums[i];
    }
    return ret;
}

__attribute__((noinline))
__attribute__((target("avx2")))
int64_t add_integers_avx2(const int64_t* nums, size_t sz) {
    int64_t ret = 0;
    for (size_t i = 0; i < sz; ++i) {
        ret += nums[i];
    }
    return ret;
}

__attribute__((target("avx2")))
int64_t add_integers_intrinc(const int64_t* nums, size_t sz) {
    size_t i = 0;
    __m256i acc = _mm256_setzero_si256();

    for (; i + 3 < sz; i += 4) {
        __m256i v = _mm256_load_si256((__m256i*)&nums[i]);
        acc = _mm256_add_epi64(acc, v);
    }

    __m128i lo = _mm256_castsi256_si128(acc);
    __m128i hi = _mm256_extracti128_si256(acc, 1);
    __m128i sum128 = _mm_add_epi64(lo, hi);
    __m128i shuffled = _mm_unpackhi_epi64(sum128, sum128);
    __m128i total = _mm_add_epi64(sum128, shuffled);
    int64_t result = _mm_cvtsi128_si64(total);

    for (; i < sz; i++) {
        result += nums[i];
    }

    return result;
}

__attribute__((noinline))
__attribute__((target("avx512f")))
int64_t add_integers_avx512(const int64_t* nums, size_t sz) {
    int64_t ret = 0;
    for (size_t i = 0; i < sz; ++i) {
        ret += nums[i];
    }
    return ret;
}

static void IntegerSumLoop(benchmark::State& state) {
    const auto nums = make_data(state.range(0));
    for (auto _ : state) {
        int64_t sum = add_integers(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoop)->RangeMultiplier(2)->Range(1 << 10, 1 << 24);

static void IntegerSumLoopAvx2(benchmark::State& state) {
    const auto nums = make_data(state.range(0));
    for (auto _ : state) {
        int64_t sum = add_integers_avx2(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoopAvx2)->RangeMultiplier(2)->Range(1 << 10, 1 << 24);


static void IntegerSumLoopAvx512(benchmark::State& state) {
    const auto nums = make_data(state.range(0));
    for (auto _ : state) {
        int64_t sum = add_integers_avx512(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoopAvx512)->RangeMultiplier(2)->Range(1 << 10, 1 << 24);

static void IntegerSumAvx2Intrin(benchmark::State& state) {
    const auto nums = make_data(state.range(0));
    for (auto _ : state) {
        int64_t sum = add_integers_intrinc(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumAvx2Intrin)->RangeMultiplier(2)->Range(1 << 10, 1 << 24);


static std::vector<int32_t> make_data_i32(size_t n) {
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int32_t> dist;
    std::vector<int32_t> v(n);
    for (auto& x : v) x = dist(rng);
    return v;
}

__attribute__((noinline))
__attribute__((optimize("O2")))
int32_t add_integers_i32(const int32_t* nums, size_t sz) {
    int32_t ret = 0;
    for (size_t i = 0; i < sz; ++i)
        ret += nums[i];
    return ret;
}

__attribute__((noinline))
__attribute__((target("avx2")))
int32_t add_integers_avx2_i32(const int32_t* nums, size_t sz) {
    int32_t ret = 0;
    for (size_t i = 0; i < sz; ++i)
        ret += nums[i];
    return ret;
}

__attribute__((noinline))
__attribute__((target("avx512f")))
int32_t add_integers_avx512_i32(const int32_t* nums, size_t sz) {
    int32_t ret = 0;
    for (size_t i = 0; i < sz; ++i)
        ret += nums[i];
    return ret;
}

__attribute__((target("avx2")))
int32_t add_integers_intrin_i32(const int32_t* nums, size_t sz) {
    size_t i = 0;
    __m256i acc = _mm256_setzero_si256();

    for (; i + 7 < sz; i += 8) {
        __m256i v = _mm256_loadu_si256((__m256i*)&nums[i]);
        acc = _mm256_add_epi32(acc, v);
    }

    // Reduce 8 x int32 in ymm → scalar
    __m128i lo = _mm256_castsi256_si128(acc);
    __m128i hi = _mm256_extracti128_si256(acc, 1);
    __m128i sum = _mm_add_epi32(lo, hi);                        // 4 x int32
    __m128i hi64 = _mm_unpackhi_epi64(sum, sum);                // duplicate high 64-bit lane
    sum = _mm_add_epi32(sum, hi64);                             // 2 x int32 in low 64 bits
    __m128i shuffled = _mm_shuffle_epi32(sum, _MM_SHUFFLE(2, 3, 0, 1));
    sum = _mm_add_epi32(sum, shuffled);
    int32_t result = _mm_cvtsi128_si32(sum);

    for (; i < sz; i++)
        result += nums[i];

    return result;
}

static void IntegerSumLoop32(benchmark::State& state) {
    const auto nums = make_data_i32(state.range(0));
    for (auto _ : state) {
        int32_t sum = add_integers_i32(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoop32)->RangeMultiplier(2)->Range(1 << 10, 1 << 24);

static void IntegerSumLoopAvx232(benchmark::State& state) {
    const auto nums = make_data_i32(state.range(0));
    for (auto _ : state) {
        int32_t sum = add_integers_avx2_i32(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoopAvx232)->RangeMultiplier(2)->Range(1 << 10, 1 << 24);

static void IntegerSumLoopAvx51232(benchmark::State& state) {
    const auto nums = make_data_i32(state.range(0));
    for (auto _ : state) {
        int32_t sum = add_integers_avx512_i32(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoopAvx51232)->RangeMultiplier(2)->Range(1 << 10, 1 << 24);

static void IntegerSumAvx2Intrin32(benchmark::State& state) {
    const auto nums = make_data_i32(state.range(0));
    for (auto _ : state) {
        int32_t sum = add_integers_intrin_i32(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumAvx2Intrin32)->RangeMultiplier(2)->Range(1 << 10, 1 << 24);

BENCHMARK_MAIN();
