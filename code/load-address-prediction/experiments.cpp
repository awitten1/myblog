#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <pthread.h>
#include <random>
#include <string>
#include <vector>

namespace {

constexpr std::size_t kArraySizeBytes = 1 << 20;
constexpr int kStride = 7;

enum class AccessPattern {
  kRandom,
  kStrided,
};

enum class CoreKind {
  kECore,
  kPCore,
};

void init_random_array(std::vector<uint64_t>& arr) {
  std::iota(arr.begin(), arr.end(), 0);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(arr.begin(), arr.end(), gen);
}

void init_strided_array(std::vector<uint64_t>& arr, int stride) {
  for (std::size_t i = 0; i < arr.size(); ++i) {
    arr[i] = (i + stride) % arr.size();
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

void run_pointer_chase(benchmark::State& state, AccessPattern pattern, CoreKind core_kind) {
  if (pthread_set_qos_class_self_np(qos_class_for_core_kind(core_kind), 0) != 0) {
    state.SkipWithError("pthread_set_qos_class_self_np failed");
    return;
  }

  const auto array_size = kArraySizeBytes / sizeof(uint64_t);
  std::vector<uint64_t> arr(array_size);

  switch (pattern) {
    case AccessPattern::kRandom:
      init_random_array(arr);
      break;
    case AccessPattern::kStrided:
      init_strided_array(arr, kStride);
      break;
  }

  const int iters = static_cast<int>(state.range(0));
  for (auto _ : state) {
    volatile uint64_t dep = 0;
    for (int i = 0; i < iters; ++i) {
      dep = arr[dep];
    }
  }

  state.SetItemsProcessed(state.iterations() * iters);
  state.SetLabel(std::string(access_pattern_name(pattern)) + "-" + core_kind_name(core_kind));
}

void BM_PointerChaseRandomECore(benchmark::State& state) {
  run_pointer_chase(state, AccessPattern::kRandom, CoreKind::kECore);
}

void BM_PointerChaseRandomPCore(benchmark::State& state) {
  run_pointer_chase(state, AccessPattern::kRandom, CoreKind::kPCore);
}

void BM_PointerChaseStridedECore(benchmark::State& state) {
  run_pointer_chase(state, AccessPattern::kStrided, CoreKind::kECore);
}

void BM_PointerChaseStridedPCore(benchmark::State& state) {
  run_pointer_chase(state, AccessPattern::kStrided, CoreKind::kPCore);
}

}  // namespace

BENCHMARK(BM_PointerChaseRandomECore)->DenseRange(100, 2000, 100);
BENCHMARK(BM_PointerChaseRandomPCore)->DenseRange(100, 2000, 100);
BENCHMARK(BM_PointerChaseStridedECore)->DenseRange(100, 2000, 100);
BENCHMARK(BM_PointerChaseStridedPCore)->DenseRange(100, 2000, 100);

BENCHMARK_MAIN();
