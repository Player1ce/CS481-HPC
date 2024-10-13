#!/bin/bash

module load intel

cmake . -DCMAKE_BUILD_TYPE=Release  -B auto-build

cd auto-build || exit

make

mv Homework3 ..

cd ..
