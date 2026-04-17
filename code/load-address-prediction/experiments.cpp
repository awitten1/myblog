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
constexpr int kReps = 100;

enum class AccessPattern {
  kRandom,
  kStrided,
  kSaRv,
};

enum class CoreKind {
  kECore,
  kPCore,
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

const char* core_kind_name(CoreKind core_kind) {
  switch (core_kind) {
    case CoreKind::kECore:
      return "ecore";
    case CoreKind::kPCore:
      return "pcore";
  }
}

qos_class_t qos_class_for_core_kind(CoreKind core_kind) {
  switch (core_kind) {
    case CoreKind::kECore:
      return QOS_CLASS_BACKGROUND;
    case CoreKind::kPCore:
      return QOS_CLASS_USER_INTERACTIVE;
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
void heat_cpu(std::chrono::milliseconds duration) {
  using clock = std::chrono::steady_clock;
  volatile uint64_t sink = 0;
  const auto deadline = clock::now() + duration;
  while (clock::now() < deadline) {
    for (int i = 0; i < 1000; ++i) {
      sink += static_cast<uint64_t>(i) * i + sink;
    }
  }
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

void run_config(AccessPattern pattern, CoreKind core_kind, const char* unit) {
  if (pthread_set_qos_class_self_np(qos_class_for_core_kind(core_kind), 0) != 0) {
    std::fprintf(stderr, "pthread_set_qos_class_self_np failed\n");
    return;
  }

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

  heat_cpu(std::chrono::milliseconds(10));

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
    const uint64_t min_s = sorted.front();
    const uint64_t p25_s = sorted[sorted.size() / 4];
    const uint64_t median_s = sorted[sorted.size() / 2];
    const uint64_t p75_s = sorted[(3 * sorted.size()) / 4];
    const uint64_t max_s = sorted.back();
    std::printf("%s,%s,%d,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%s\n",
                access_pattern_name(pattern),
                core_kind_name(core_kind),
                iters,
                static_cast<unsigned long long>(min_s),
                static_cast<unsigned long long>(p25_s),
                static_cast<unsigned long long>(median_s),
                static_cast<unsigned long long>(p75_s),
                static_cast<unsigned long long>(max_s),
                static_cast<unsigned long long>(first_s),
                static_cast<unsigned long long>(last_s),
                unit);
    std::fflush(stdout);
  }
}

}  // namespace

int main() {
  g_use_kperf = g_events.setup_performance_counters();
  const char* unit = g_use_kperf ? "cycles" : "ns";
  std::fprintf(stderr, "timing unit: %s\n", unit);
  std::printf("pattern,core_kind,iters,min,p25,median,p75,max,first,last,unit\n");
  const auto run = [unit](AccessPattern p, CoreKind c) {
    run_config(p, c, unit);
  };
  run(AccessPattern::kRandom, CoreKind::kECore);
  run(AccessPattern::kRandom, CoreKind::kPCore);
  run(AccessPattern::kStrided, CoreKind::kECore);
  run(AccessPattern::kStrided, CoreKind::kPCore);
  run(AccessPattern::kSaRv, CoreKind::kECore);
  run(AccessPattern::kSaRv, CoreKind::kPCore);
  return 0;
}
