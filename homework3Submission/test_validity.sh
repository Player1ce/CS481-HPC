#!/bin/bash

module load intel

directory='/scratch/ualclsd0201/'
test_file='output_5x5_1_0.txt'

numThreads=1;
./Homework3 5 1 "$numThreads" "$directory"

numThreads=2;
./Homework3 5 1 "$numThreads" "$directory" "$test_file"

numThreads=3;
./Homework3 5 1 "$numThreads" "$directory" "$test_file"

numThreads=4;
./Homework3 5 1 "$numThreads" "$directory" "$test_file"

numThreads=5;
./Homework3 5 1 "$numThreads" "$directory" "$test_file"
