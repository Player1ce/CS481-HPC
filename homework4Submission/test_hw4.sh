#!/bin/bash

module load intel
module load openmpi/4.1.4-gcc11

directory='/scratch/ualclsd0201/'
#directory='validation/'
test_file='output_5000x5000_5000_0.txt'

# Generate test file
numThreads=20
./Homework3 5000 5000 "$numThreads" "$directory"

numThreads=1;
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=2;

mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=4;

mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=8;

mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=10;

mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=16;

mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"

numThreads=20;

mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"
mpirun -n "$numThreads" Homework4 5000 5000 "$numThreads" "$directory" "$test_file"

for fileNum in {1..21}
do
  diff -s "$directory"output_5000x5000_5000_"$fileNum".txt "$directory""$test_file"
done