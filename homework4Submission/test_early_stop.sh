#!/bin/bash

source /apps/profiles/modules_asax.sh.dyn

module  load  openmpi/4.1.4-gcc11
module load intel

directory='/scratch/ualclsd0201/'
#directory='validation/'
test_file='output_5x5_10_0.txt'

numThreads=1;
mpirun -n "$numThreads" Homework4 5 10 "$numThreads" "$directory"

numThreads=2;
mpirun -n "$numThreads" Homework4 5 10 "$numThreads" "$directory" "$test_file"


numThreads=3;
mpirun -n "$numThreads" Homework4 5 10 "$numThreads" "$directory" "$test_file"


numThreads=4;
mpirun -n "$numThreads" Homework4 5 10 "$numThreads" "$directory" "$test_file"


numThreads=5;
mpirun -n "$numThreads" Homework4 5 10 "$numThreads" "$directory" "$test_file"

for fileNum in {1..4}
do
  diff -s "$directory"output_5x5_10_"$fileNum".txt "$directory$test_file"
done
