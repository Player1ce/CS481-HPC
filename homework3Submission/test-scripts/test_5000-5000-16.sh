#!/bin/bash

module load intel


numThreads=16;
directory='/scratch/ualclsd0201/'
test_file='output_5000x5000_5000_0.txt'

./../Homework3 5000 5000 "$numThreads" "$directory" "$test_file"