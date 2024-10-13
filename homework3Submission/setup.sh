#!/usr/bin/env bash

module load intel
module load cmake

mkdir build
cd build

mkdir debug
mkdir release

cd release
cmake -DCMAKE_BUILD_TYPE=Release ../..
cd ..

cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
cd ..
