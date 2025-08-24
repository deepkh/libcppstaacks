#!/bin/bash

OUT=Out

# Generate Makefile from CMake
if [ ! -d Out ];then
mkdir $OUT
pushd $OUT
cmake -DCMAKE_BUILD_TYPE=Debug ../
popd
fi

# Build
pushd $OUT
make -j4
popd