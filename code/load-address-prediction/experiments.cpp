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
};

enum class CoreKind {
  kECore,
  kPCore,
};

void init_random_array(std::vector<int>& arr) {
  std::iota(arr.begin(), arr.end(), 0);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(arr.begin(), arr.end(), gen);
}

void init_strided_array(std::vector<int>& arr, int stride) {
  for (std::size_t i = 0; i < arr.size(); ++i) {
    arr[i] = static_cast<int>(i) + stride;
  }
}

const char* access_pattern_name(AccessPattern pattern) {
  switch (pattern) {
    case AccessPattern::kRandom:
      return "random";
    case AccessPattern::kStrided:
      return "strided";
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

uint64_t measure_once(volatile int* arr, int iters) {
  volatile int dep = 0;
  for (int i = 0; i < iters; ++i) {
    dep = arr[dep];
  }

  dep = 0;
  if (g_use_kperf) {
    const auto before = g_events.get_counters();
    for (int i = 0; i < iters; ++i) {
      dep = arr[dep];
    }
    const auto after = g_events.get_counters();
    return static_cast<uint64_t>(after.cycles - before.cycles);
  }

  const auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < iters; ++i) {
    dep = arr[dep];
  }
  const auto end = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void run_config(AccessPattern pattern, CoreKind core_kind, const char* unit) {
  if (pthread_set_qos_class_self_np(qos_class_for_core_kind(core_kind), 0) != 0) {
    std::fprintf(stderr, "pthread_set_qos_class_self_np failed\n");
    return;
  }

  const auto array_size = kArraySizeBytes / sizeof(int);
  std::vector<int> arr(array_size);

  switch (pattern) {
    case AccessPattern::kRandom:
      init_random_array(arr);
      break;
    case AccessPattern::kStrided:
      init_strided_array(arr, kStride);
      break;
  }

  heat_cpu(std::chrono::milliseconds(10));

  std::vector<uint64_t> samples(kReps);
  for (int iters = kMinIters; iters <= kMaxIters; iters += kIterStep) {
    for (int r = 0; r < kReps; ++r) {
      samples[r] = measure_once(arr.data(), iters);
    }
    const uint64_t min_sample = *std::min_element(samples.begin(), samples.end());
    std::printf("%s,%s,%d,%llu,%s\n",
                access_pattern_name(pattern),
                core_kind_name(core_kind),
                iters,
                static_cast<unsigned long long>(min_sample),
                unit);
    std::fflush(stdout);
  }
}

}  // namespace

int main() {
  g_use_kperf = g_events.setup_performance_counters();
  const char* unit = g_use_kperf ? "cycles" : "ns";
  std::fprintf(stderr, "timing unit: %s\n", unit);
  std::printf("pattern,core_kind,iters,runtime,unit\n");
  const auto run = [unit](AccessPattern p, CoreKind c) {
    run_config(p, c, unit);
  };
  run(AccessPattern::kRandom, CoreKind::kECore);
  run(AccessPattern::kRandom, CoreKind::kPCore);
  run(AccessPattern::kStrided, CoreKind::kECore);
  run(AccessPattern::kStrided, CoreKind::kPCore);
  return 0;
}
