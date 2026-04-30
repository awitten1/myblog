#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

deps_dir="$(pwd)/deps"
install_dir="$deps_dir/install_dir"

if [ ! -d "$install_dir" ]; then
    mkdir -p "$deps_dir"
    (cd "$deps_dir" && ../../utils/install-gbench.sh) 1>&2
fi

if [ ! -f build/build.ninja ]; then
    cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
        -Dbenchmark_DIR="$install_dir/lib/cmake/benchmark" 1>&2
fi
cmake --build build 1>&2

if [ -n "${BENCH_CSV:-}" ]; then
    ./build/bench_add --benchmark_perf_counters=INSTRUCTIONS,CYCLES \
        --benchmark_format=csv 2>/dev/null
else
    ./build/bench_add --benchmark_perf_counters=INSTRUCTIONS,CYCLES
fi
