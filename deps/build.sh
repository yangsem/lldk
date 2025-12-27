#!/bin/bash

set -e

WORKDIR=$(cd $(dirname $0); pwd)
DEPSDIR=$WORKDIR/deps

BUILD_LOG=$WORKDIR/build.log

CXX_STANDARD=11
if [ "$1" != "" ]; then
    CXX_STANDARD=$1
fi

BUILD_TARGET=all
if [ "$2" != "" ]; then
    BUILD_TARGET=$2
fi

build_help()
{
    echo "Usage: $0 [CXX_STANDARD] [BUILD_TARGET]"
    echo "CXX_STANDARD: The C++ standard to use"
    echo "BUILD_TARGET: The target to build"
}

build_googletest()
{
    echo "================================================"
    echo "Building googletest"
    echo "================================================"

    rm -rf $DEPSDIR/googletest
    mkdir -p $DEPSDIR/googletest

    cd $WORKDIR/googletest
    mkdir -p build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=$DEPSDIR/googletest -DCMAKE_CXX_STANDARD=$CXX_STANDARD >> $BUILD_LOG 2>&1
    make -j8 >> $BUILD_LOG 2>&1
    make install >> $BUILD_LOG 2>&1

    echo "================================================"
    echo "Build googletest done"
    echo "================================================"
}

main()
{
    mkdir -p $DEPSDIR

    case "$BUILD_TARGET" in
        "googletest")
            build_googletest
            ;;
        "all")
            build_googletest
            ;;
        *)
            build_help
            ;;
    esac
}

main