#include <algorithm>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "setup.h"
#include "apple_arm_events.h"

static AppleEvents g_events;
static bool g_use_kperf = false;

static const size_t kArraySizeBytes = 1 << 15;
static const int kMinIters = 10;
static const int kMaxIters = 1000;
static const int kIterStep = 10;
static const int kReps = 100;

enum AccessPattern {
  PATTERN_RANDOM,
  PATTERN_STRIDED,
  PATTERN_SA_RV,
};

static const char* access_pattern_name(enum AccessPattern pattern) {
  switch (pattern) {
    case PATTERN_RANDOM:  return "random";
    case PATTERN_STRIDED: return "strided";
    case PATTERN_SA_RV:   return "sa_rv";
  }
  return "unknown";
}

static int current_cpu(void) {
  size_t cpu = 0;
  pthread_cpu_number_np(&cpu);
  return (int)cpu;
}

static uint64_t get_time(void) {
  if (g_use_kperf) {
    return (uint64_t)g_events.get_counters().cycles;
  }
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static uint64_t time_chase(volatile int* arr, int iters) {
  volatile int dep = 0;
  for (int i = 0; i < iters; ++i) {
    dep = arr[dep];
  }

  dep = 0;
  uint64_t start = get_time();
  for (int i = 0; i < iters; ++i) {
    dep = arr[dep];
  }
  uint64_t end = get_time();
  return end - start;
}

static uint64_t time_sa_rv(volatile int* arr, int iters) {
  volatile int dep = 0;
  for (int i = 0; i < iters; ++i) {
    const int v = arr[dep];
    dep += v < kStride ? v : kStride;
  }

  dep = 0;
  uint64_t start = get_time();
  for (int i = 0; i < iters; ++i) {
    const int v = arr[dep];
    dep += v < kStride ? v : kStride;
  }
  uint64_t end = get_time();
  return end - start;
}

static void run(enum AccessPattern pattern, const char* unit,
                const char* core_kind, bool write) {
  const size_t array_size = kArraySizeBytes / sizeof(int);
  int* arr = new int[array_size];

  switch (pattern) {
    case PATTERN_RANDOM:  init_random_array(arr, array_size); break;
    case PATTERN_STRIDED: init_strided_array(arr, array_size); break;
    case PATTERN_SA_RV:   init_sa_rv_array(arr, array_size); break;
  }

  uint64_t* samples = new uint64_t[kReps];

  for (int iters = kMinIters; iters <= kMaxIters; iters += kIterStep) {
    int cpu = current_cpu();
    for (int r = 0; r < kReps; ++r) {
      if (pattern == PATTERN_SA_RV) {
        samples[r] = time_sa_rv(arr, iters);
      } else {
        samples[r] = time_chase(arr, iters);
      }
    }
    std::sort(samples, samples + kReps);
    const uint64_t median_s = samples[kReps / 2];
    if (write) {
      printf("%s,%s,%d,%d,%llu,%s\n",
             access_pattern_name(pattern),
             core_kind,
             cpu,
             iters,
             (unsigned long long)median_s,
             unit);
      fflush(stdout);
    }
  }

  delete[] samples;
  delete[] arr;
}

int main(void) {
  g_use_kperf = g_events.setup_performance_counters();
  const char* unit = g_use_kperf ? "cycles" : "ns";
  fprintf(stderr, "timing unit: %s\n", unit);
  printf("pattern,core_kind,cpu,iters,median,unit\n");

  if (pthread_set_qos_class_self_np(QOS_CLASS_BACKGROUND, 0) != 0) {
    fprintf(stderr, "failed to set qos class\n");
    exit(EXIT_FAILURE);
  }
  run(PATTERN_SA_RV,   unit, "ecore", false);
  run(PATTERN_SA_RV,   unit, "ecore", true);
  run(PATTERN_STRIDED, unit, "ecore", false);
  run(PATTERN_STRIDED, unit, "ecore", true);
  run(PATTERN_RANDOM,  unit, "ecore", false);
  run(PATTERN_RANDOM,  unit, "ecore", true);

  if (pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0) != 0) {
    fprintf(stderr, "failed to set qos class\n");
    exit(EXIT_FAILURE);
  }
  run(PATTERN_SA_RV,   unit, "pcore", false);
  run(PATTERN_SA_RV,   unit, "pcore", true);
  run(PATTERN_STRIDED, unit, "pcore", false);
  run(PATTERN_STRIDED, unit, "pcore", true);
  run(PATTERN_RANDOM,  unit, "pcore", false);
  run(PATTERN_RANDOM,  unit, "pcore", true);
  return 0;
}
