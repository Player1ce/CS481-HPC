#!/usr/bin/env bash

set -euxo pipefail

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
