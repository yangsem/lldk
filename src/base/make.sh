#!/bin/bash

set -e 

if [ "$1" == "clean" ]; then
    rm -rf build
    mkdir build
    # 去除第一个参数
    shift
fi

cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=OFF

# 如果是以 -j 开头，则设置并去除第一个参数
if [ "$1" == "-j*" ]; then
    make "$1"
    shift
else
    make
fi

cp compile_commands.json ../