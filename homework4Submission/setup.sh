#!/usr/bin/env bash

module load intel
module  load  openmpi/4.1.4-gcc11
module load cmake

mkdir build
cd build || exit

mkdir debug
mkdir release

cd release || exit
cmake -DCMAKE_BUILD_TYPE=Release ../..
cd ..

cd debug || exit
cmake -DCMAKE_BUILD_TYPE=Debug ../..
cd ..
