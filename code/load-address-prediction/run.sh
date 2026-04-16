#!/bin/bash

set -euo pipefail

cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build

./build/load_address_prediction_bench \
  --benchmark_format=csv 2>/dev/null | tee results.csv

