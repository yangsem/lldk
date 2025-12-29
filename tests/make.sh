#!/bin/bash

set -e

if [ "$1" == "clean" ]; then
    rm -rf build
    mkdir build
fi

mkdir -p build
cd build
cmake ..
make -j8
