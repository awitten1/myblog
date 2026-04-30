#include <benchmark/benchmark.h>

#include <sys/mman.h>

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

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

extern "C" int64_t add_integers_asm(const int64_t* nums, size_t sz);
extern "C" int64_t add_integers_asm_repeat(const int64_t* nums, size_t sz, size_t repeat);

__attribute__((noinline))
__attribute__((target("avx2")))
int64_t add_integers_cpp_repeat(const int64_t* nums, size_t sz, size_t repeat) {
    int64_t ret = 0;
    for (size_t r = 0; r < repeat; ++r) {
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

static inline __attribute__((always_inline))
__attribute__((target("avx2")))
int64_t add_integers_avx2_inline(const int64_t* nums, size_t sz) {
    int64_t ret = 0;
    for (size_t i = 0; i < sz; ++i) {
        ret += nums[i];
    }
    return ret;
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
BENCHMARK(IntegerSumLoop)->Range(1 << 10, 1 << 20);

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

static void IntegerSumLoopAsm(benchmark::State& state) {
    const auto nums = make_data(state.range(0));
    for (auto _ : state) {
        int64_t sum = add_integers_asm(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoopAsm)->Range(1 << 10, 1 << 24);

static void IntegerSumLoopAsmRepeat(benchmark::State& state) {
    const auto nums = make_data(1024);
    const size_t repeat = state.range(0);
    for (auto _ : state) {
        int64_t sum = add_integers_asm_repeat(nums.data(), nums.size(), repeat);
        benchmark::DoNotOptimize(sum);
    }
    const size_t hot_iters_per_call = (1024 / 4) * repeat;
    state.counters["hot_iters"] = hot_iters_per_call;
}
BENCHMARK(IntegerSumLoopAsmRepeat)->Arg(1)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);

static void IntegerSumLoopAvx2Mmap(benchmark::State& state) {
    size_t n = state.range(0);
    int64_t* nums = mmap_data(n);
    for (auto _ : state) {
        int64_t sum = add_integers_avx2(nums, n);
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * n);
    state.counters["size"] = n;
    munmap(nums, n * sizeof(int64_t));
}
BENCHMARK(IntegerSumLoopAvx2Mmap)->Range(1 << 10, 1 << 24);

__attribute__((target("avx2")))
static void IntegerSumLoopAvx2Inline(benchmark::State& state) {
    const auto nums = make_data(state.range(0));
    for (auto _ : state) {
        int64_t sum = add_integers_avx2_inline(nums.data(), nums.size());
        benchmark::DoNotOptimize(sum);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
    state.counters["size"] = state.range(0);
}
BENCHMARK(IntegerSumLoopAvx2Inline)->Range(1 << 10, 1 << 24);

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

BENCHMARK_MAIN();
