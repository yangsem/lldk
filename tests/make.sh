#!/bin/bash

set -e

if [ "$1" == "clean" ]; then
    rm -rf build
    mkdir build
    shift
fi

mkdir -p build
cd build
cmake ..

if [ "$1" == "-j*" ]; then
    make "$1"
    shift
else
    make
fi
