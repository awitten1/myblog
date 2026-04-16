#!/bin/bash

set -euo pipefail

cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build

./build/load_address_prediction_bench | tee results.csv
