#!/bin/bash

source /apps/profiles/modules_asax.sh.dyn

module load cuda

directory='/scratch/ualclsd0201/'
#directory='validation/'
test_file='output_5x5_10_0.txt'

numThreads=1;
mpirun -n "$numThreads" Homework5 5 10 "$numThreads" "$directory"
