#include <benchmark/benchmark.h>

#include <sys/mman.h>

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>
#include <immintrin.h>

static int64_t* mmap_data(size_t n) {
    int64_t* p = (int64_t*)mmap(nullptr, n * sizeof(int64_t),
                                PROT_READ | PROT_WRITE,
                                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int64_t> dist;
    for (size_t i = 0; i < n; ++i) p[i] = dist(rng);
    return p;
}

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

// extern "C" int64_t add_integers_asm(const int64_t* nums, size_t sz);
// extern "C" int64_t add_integers_asm_repeat(const int64_t* nums, size_t sz, size_t repeat);

__attribute__((noinline))
__attribute__((target("avx2")))
int64_t add_integers_cpp_repeat(const int64_t* nums, size_t sz, size_t repeat) {
    int64_t ret = 0;
    for (size_t i = 0; i < repeat; ++i) {
        ret += nums[i & (sz - 1)];
    }
    return ret;
}

__attribute__((noinline))
__attribute__((target("avx2")))
int64_t add_integers_cpp_repeat_avx2(const int64_t* nums, size_t sz, size_t total) {
    int64_t ret = 0;
    const size_t outer = total / sz;
    for (size_t r = 0; r < outer; ++r) {
        for (size_t i = 0; i < sz; ++i) {
            ret += nums[i];
        }
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
BENCHMARK(IntegerSumLoop)->Range(1 << 10, 1 << 24);

static void IntegerSumLoopAvx2(benchmark::State& state) {
    const auto nums = make_data(state.range(0));
    for (auto _ : state) {
        int64_t sum = add_integers_avx2(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoopAvx2)->Range(1 << 10, 1 << 24);


static void IntegerSumLoopAvx512(benchmark::State& state) {
    const auto nums = make_data(state.range(0));
    for (auto _ : state) {
        int64_t sum = add_integers_avx512(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoopAvx512)->Range(1 << 10, 1 << 24);

static void IntegerSumAvx2Intrin(benchmark::State& state) {
    const auto nums = make_data(state.range(0));
    for (auto _ : state) {
        int64_t sum = add_integers_intrinc(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumAvx2Intrin)->Range(1 << 10, 1 << 24);


BENCHMARK_MAIN();
