#!/bin/bash

module load intel

numThreads=1;
directory='/scratch/ualclsd0201/'
test_file='output_5000x5000_5000_0.txt'

./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=2;

./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=4;

./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=8;

./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=10;

./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=16;

./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=20;

./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./Homework3 5000 5000 "$numThreads" "$directory" "$test_file"