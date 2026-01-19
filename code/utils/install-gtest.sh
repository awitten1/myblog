#!/bin/bash

set -eux

dir=$(pwd)

install_dir="$dir/install_dir"
build_dir="$dir/build_dir"
source_dir="$dir/source_dir"

mkdir -p $install_dir
mkdir -p $build_dir
mkdir -p $source_dir

install_google_test() {
    local gtest_version=v1.14.0

    pushd $source_dir

    if [ ! -d "googletest" ]; then
        git clone --branch ${gtest_version} https://github.com/google/googletest.git
    fi

    cmake -B $build_dir/googletest -S googletest -DCMAKE_INSTALL_PREFIX=$install_dir \
        -DCMAKE_BUILD_TYPE=Release -G Ninja

    cmake --build $build_dir/googletest -j8
    cmake --install $build_dir/googletest
    popd
}

install_google_test
