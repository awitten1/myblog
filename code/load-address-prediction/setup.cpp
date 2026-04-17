#include <random>

extern const int kStride = 32 / sizeof(int);

void init_random_array(int* arr, int array_size) {
  std::iota(arr, arr + array_size, 0);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::shuffle(arr, arr + array_size, gen);
}

void init_strided_array(int* arr, int array_sz) {
  for (std::size_t i = 0; i < array_sz; i+=kStride) {
    arr[i] = i + kStride;
  }
}

void init_sa_rv_array(int* arr, int array_size) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(0, 8);
  for (int i = 0; i < array_size; ++i) {
    arr[i] = dist(gen) * kStride;
  }
}