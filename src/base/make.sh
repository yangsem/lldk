#!/bin/bash

set -e 

if [ "$1" == "clean" ]; then
    rm -rf build
    mkdir build
fi

cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=OFF
make -j$(nproc)
cp compile_command.json ../compile_command.json