#!/bin/bash

module load intel
module  load  openmpi/4.1.4-gcc11
module load cmake

cmake . -DCMAKE_BUILD_TYPE=Release  -B auto-build

cd auto-build || exit

make

mv Homework3 ..
mv Homework4 ..

cd ..
