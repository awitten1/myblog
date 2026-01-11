#!/bin/bash

set -eux

mkdir -p results
mkdir -p results/amdzen5
mkdir -p results/amdzen2
mkdir -p results/macbookpro
mkdir -p results/raspi

if [ $(uname -s) = 'Linux' ]; then
    cpu_model=$(cat /proc/cpuinfo | grep 'model name' | head -n 1 | cut -d ':' -f2)
    if [[ ${cpu_model} == *'9900X'* ]]; then
        ./build/main --benchmark_format=csv \
            --benchmark_perf_counters=instructions,cycles > results/amdzen5/cycles.csv
        ./build/main --benchmark_format=csv \
            --benchmark_perf_counters=branches,branch-misses > results/amdzen5/branches.csv
        # See document Performance Monitor Counters for AMD Family 1Ah Model 50h-57h Processors
        # also see https://github.com/torvalds/linux/blob/v6.18/tools/perf/pmu-events/arch/x86/amdzen5/pipeline.json
        # which contains much of the same info.
        #
        # The Bad Speculation metric is (Event[4307AA] – Event[4300C1]) / Total Dispatch Slots
        # Total Dispatch Slots = 8 * Event[430076].
        # Up to 8 instructions can be dispatched in a single cycle.
        # Macro-ops Dispatched = 4307AA
        # Macro-ops Retired = 4300C1
        ./build/main --benchmark_format=csv \
                   --benchmark_perf_counters=perf_raw::r4307AA,perf_raw::r4300C1,perf_raw::r430076 \
                   > results/amdzen5/bad_speculation.csv

        # 100431EA0 = backend bound = dispatch slots that are unused because of backend stalls.
        # 1004301A0 = dispatch slots that are unused becausee frontend didn't supply enough macro-ops
        ./build/main --benchmark_format=csv \
                   --benchmark_perf_counters=perf_raw::r430076,perf_raw::r100431EA0,perf_raw::r1004301A0 \
                   > results/amdzen5/frontend_backend_bound.csv


    elif [[ ${cpu_model} == *'3995WX'* ]]; then
         ./build/main --benchmark_format=csv \
            --benchmark_perf_counters=instructions,cycles > results/amdzen2/cycles.csv
        ./build/main --benchmark_format=csv \
            --benchmark_perf_counters=branches,branch-misses > results/amdzen2/branches.csv
    elif [[ $(uname -a) == *'raspi'* ]]; then
        ./build/main --benchmark_format=csv \
            --benchmark_perf_counters=instructions,cycles > results/raspi/cycles.csv
        ./build/main --benchmark_format=csv \
            --benchmark_perf_counters=branches,branch-misses > results/raspi/branches.csv
    fi
fi

if [ $(uname -s ) = 'Darwin' ]; then
    ./build/main --benchmark_format=csv > results/macbookpro/results.csv
fi
