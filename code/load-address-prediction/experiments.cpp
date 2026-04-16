
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string>
#include <random>

void init_random_array(uint64_t* arr, int array_sz) {
  std::iota(arr, arr + array_sz, 0);
  std::random_device rd;
  std::mt19937 gen(rd());

  for (int i = array_sz-1; i >= 1; --i) {
    std::uniform_int_distribution<> dist(0, i);
    int j = dist(gen);
    std::swap(arr[i],arr[j]);
  }
}

void init_strided_array(uint64_t* arr, int array_size, int stride) {
  for (int i = 0; i < array_size; ++i) {
    arr[i] = (i+stride) % array_size;
  }
}

void print_error_message() {
  fprintf(stderr, "./lap <array size bytes>");
  exit(EXIT_FAILURE);
}

std::chrono::duration<float> run_experiment(volatile uint64_t* arr, int iters) {
  volatile uint64_t dep = 0;
  for (int i = 0; i < iters; ++i) {
    dep = arr[dep];
  }

  auto t1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < iters; ++i) {
    dep = arr[dep];
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> time(t2 - t1);

  return time;
}


int main(int argc, char** argv) {
  if (argc != 2) {
    print_error_message();
  }
  int array_size_bytes = std::stoi(argv[1]);
  int array_size = array_size_bytes / sizeof(long);


  std::cout << "mode,iterations,time_ms" << std::endl;
  for (int iters = 100; iters <= 2000; iters += 100) {
    uint64_t* arr = new uint64_t[array_size];
    init_random_array(arr, array_size);
    std::chrono::duration<double, std::milli> time = run_experiment(arr, iters);
    std::cout << "random," << iters << "," << time.count() << std::endl;

    init_strided_array(arr, array_size,7);
    time = run_experiment(arr, iters);
    std::cout << "strided," << iters << "," << time.count() << std::endl;
    delete[] arr;
  }


  return 0;
}