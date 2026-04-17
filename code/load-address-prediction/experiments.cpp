#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include "setup.h"

#include "apple_arm_events.h"

namespace {

AppleEvents g_events;
bool g_use_kperf = false;

const size_t kArraySizeBytes = 1 << 15;
const int kMinIters = 10;
const int kMaxIters = 1000;
const int kIterStep = 10;
const int kReps = 100;

enum class AccessPattern {
  kRandom,
  kStrided,
  kSaRv,
};

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

uint64_t measure_inc(volatile int* arr, int iters) {
  volatile int dep = 0;
  for (int i = 0; i < iters; ++i) {
    const int v = arr[dep];
    dep += v;
  }

  dep = 0;
  auto start = get_time();
  for (int i = 0; i < iters; ++i) {
    const int v = arr[dep];
    dep += v;
  }
  auto end = get_time();
  return end - start;
}

void run(AccessPattern pattern, const char* unit, const char* core_kind, bool write) {
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
    if (write) {
      std::printf("%s,%s,%d,%llu,%s\n",
                  access_pattern_name(pattern),
                  core_kind,
                  iters,
                  static_cast<unsigned long long>(median_s),
                  unit);
      std::fflush(stdout);
    }
  }

  delete[] arr;
}

}

int main() {
  g_use_kperf = g_events.setup_performance_counters();
  const char* unit = g_use_kperf ? "cycles" : "ns";
  std::fprintf(stderr, "timing unit: %s\n", unit);
  std::printf("pattern,core_kind,iters,median,unit\n");

  if (pthread_set_qos_class_self_np(QOS_CLASS_BACKGROUND, 0) != 0) {
    fprintf(stderr, "failed to set qos class\n");
    exit(EXIT_FAILURE);
  }
  run(AccessPattern::kStrided, unit, "ecore", true);
  run(AccessPattern::kRandom, unit, "ecore", true);
  run(AccessPattern::kSaRv, unit, "ecore", true);

  if (pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0) != 0) {
    fprintf(stderr, "failed to set qos class\n");
    exit(EXIT_FAILURE);
  }
  run(AccessPattern::kSaRv, unit, "pcore", false);
  run(AccessPattern::kSaRv, unit, "pcore", true);
  run(AccessPattern::kStrided, unit, "pcore", false);
  run(AccessPattern::kStrided, unit, "pcore", true);
  run(AccessPattern::kRandom, unit, "pcore", false);
  run(AccessPattern::kRandom, unit, "pcore", true);
  return 0;
}
