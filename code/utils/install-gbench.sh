#!/bin/bash

set -eux

dir=$(pwd)

install_dir="$dir/install_dir"
build_dir="$dir/build_dir"
source_dir="$dir/source_dir"

mkdir -p $install_dir
mkdir -p $build_dir
mkdir -p $source_dir

install_libpfm() {
    pushd $source_dir
    git clone --branch v4.13.0 https://github.com/wcohen/libpfm4.git
    pushd libpfm4
    make PREFIX=$install_dir install
    popd
    popd
}

install_google_benchmark() {
    local gbench_version=v1.9.4

    pushd $source_dir

    if [ ! -d "benchmark" ]; then
        git clone --branch ${gbench_version} https://github.com/google/benchmark.git
    fi

    local enable_libpfm=OFF
    if [ $(uname -s) = 'Linux' ]; then
        enable_libpfm=ON
        install_libpfm
    fi
    cmake -B $build_dir/benchmark -S benchmark -DCMAKE_INSTALL_PREFIX=$install_dir \
        -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON -G Ninja \
        -DBENCHMARK_ENABLE_LIBPFM=${enable_libpfm} -DCMAKE_PREFIX_PATH=$install_dir

    cmake --build $build_dir/benchmark -j8
    cmake --install $build_dir/benchmark
    popd
}

install_google_benchmark






