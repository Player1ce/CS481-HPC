#!/bin/bash

source /apps/profiles/modules_asax.sh.dyn

#module  load  openmpi/4.1.4-gcc11
module load intel

module load cmake
module load cuda

cmake . -DCMAKE_BUILD_TYPE=Release  -B auto-build

cd auto-build || exit

make

mv Homework3 ..
mv Homework5 ..

cd ..
