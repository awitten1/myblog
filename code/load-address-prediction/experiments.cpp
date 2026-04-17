#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <numeric>
#include <pthread.h>
#include <random>
#include <vector>

#include "apple_arm_events.h"

namespace {

AppleEvents g_events;
bool g_use_kperf = false;

constexpr std::size_t kArraySizeBytes = 1 << 15;
constexpr int kStride = 32 / sizeof(int);
constexpr int kMinIters = 10;
constexpr int kMaxIters = 1000;
constexpr int kIterStep = 10;
constexpr int kReps = 1000;

enum class AccessPattern {
  kRandom,
  kStrided,
  kSaRv,
};

__attribute__((noinline))
void init_random_array(int* arr, int array_size) {
  std::iota(arr, arr + array_size, 0);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(arr, arr + array_size, gen);
}

__attribute__((noinline))
void init_strided_array(int* arr, int array_sz) {
  for (std::size_t i = 0; i < array_sz; i+=kStride) {
    arr[i] = i + kStride;
  }
}

__attribute__((noinline))
void init_sa_rv_array(int* arr, int array_size) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(1, 128);
  for (std::size_t i = 0; i < array_size; ++i) {
    arr[i] = dist(gen) * kStride;
  }
}

const char* access_pattern_name(AccessPattern pattern) {
  switch (pattern) {
    case AccessPattern::kRandom:
      return "random";
    case AccessPattern::kStrided:
      return "strided";
    case AccessPattern::kSaRv:
      return "sa_rv";
  }
}

uint64_t get_time() {
  if (g_use_kperf) {
    return g_events.get_counters().cycles;
  }
  return std::chrono::duration_cast<std::chrono::nanoseconds>
    (std::chrono::steady_clock::now().time_since_epoch()).count();
}

__attribute__((noinline))
uint64_t measure_chase(volatile int* arr, int iters) {
  volatile int dep = 0;
  for (int i = 0; i < iters; ++i) {
    dep = arr[dep];
  }

  dep = 0;
  auto start = get_time();
  for (int i = 0; i < iters; ++i) {
    dep = arr[dep];
  }
  auto end = get_time();
  return end - start;
}

__attribute__((noinline))
uint64_t measure_sa_rv(volatile int* arr, int iters) {
  volatile int dep = 0;
  for (int i = 0; i < iters; ++i) {
    const int v = arr[dep];
    dep += v < kStride ? v : kStride;
  }

  dep = 0;
  auto start = get_time();
  for (int i = 0; i < iters; ++i) {
    const int v = arr[dep];
    dep += v < kStride ? v : kStride;
  }
  auto end = get_time();
  return end - start;
}

void run(AccessPattern pattern, const char* unit, const char* core_kind) {
  const auto array_size = kArraySizeBytes / sizeof(int);
  int* arr = new int[array_size];

  switch (pattern) {
    case AccessPattern::kRandom:
      init_random_array(arr, array_size);
      break;
    case AccessPattern::kStrided:
      init_strided_array(arr, array_size);
      break;
    case AccessPattern::kSaRv:
      init_sa_rv_array(arr, array_size);
      break;
  }

  std::vector<uint64_t> samples(kReps);
  for (int iters = kMinIters; iters <= kMaxIters; iters += kIterStep) {
    for (int r = 0; r < kReps; ++r) {
      if (pattern == AccessPattern::kSaRv) {
        samples[r] = measure_sa_rv(arr, iters);
      } else {
        samples[r] = measure_chase(arr, iters);
      }
    }
    const uint64_t first_s = samples.front();
    const uint64_t last_s = samples.back();
    std::vector<uint64_t> sorted = samples;
    std::sort(sorted.begin(), sorted.end());
    const uint64_t median_s = sorted[sorted.size() / 2];
    std::printf("%s,%s,%d,%llu,%s\n",
                access_pattern_name(pattern),
                core_kind,
                iters,
                static_cast<unsigned long long>(median_s),
                unit);
    std::fflush(stdout);
  }
}

}  // namespace

int main() {
  g_use_kperf = g_events.setup_performance_counters();
  const char* unit = g_use_kperf ? "cycles" : "ns";
  std::fprintf(stderr, "timing unit: %s\n", unit);
  std::printf("pattern,core_kind,iters,median,unit\n");

  pthread_set_qos_class_self_np(QOS_CLASS_BACKGROUND, 0);
  run(AccessPattern::kStrided, unit, "ecore");
  run(AccessPattern::kRandom, unit, "ecore");
  run(AccessPattern::kSaRv, unit, "ecore");

  pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
  run(AccessPattern::kStrided, unit, "pcore");
  run(AccessPattern::kRandom, unit, "pcore");
  run(AccessPattern::kSaRv, unit, "pcore");
  return 0;
}
