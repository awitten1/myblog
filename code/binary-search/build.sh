#!/bin/bash

set -eux

dir=$(dirname $(realpath $0))

pushd $dir

if [ ! -d ./install_dir ]; then
    ${dir}/../utils/install-gbench.sh
fi

cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -G Ninja -DMARCH=native
cmake --build build -j3


popd