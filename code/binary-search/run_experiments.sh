#!/bin/bash

set -eux

mkdir -p results
mkdir -p results/amdzen5
mkdir -p results/macbookpro

if [ $(uname -s) = 'Linux' ]; then
    cpu_model=$(cat /proc/cpuinfo | grep 'model name' | head -n 1 | cut -d ':' -f2)
    target_model='9900X'
    if [[ ${cpu_model} == *${target_model}* ]]; then
        ./build/main --benchmark_format=csv \
            --benchmark_perf_counters=instructions,cycles > results/amdzen5/cycles.csv
        ./build/main --benchmark_format=csv \
            --benchmark_perf_counters=branches,branch-misses > results/amdzen5/branches.csv
    fi
fi

if [ $(uname -s ) = 'Darwin' ]; then
    ./build/main --benchmark_format=csv > results/macbookpro/results.csv
fi
