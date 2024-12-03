#!/bin/bash

source /apps/profiles/modules_asax.sh.dyn
#
#module  load  openmpi/4.1.4-gcc11
#module load intel

module load cuda

#directory='/scratch/ualclsd0201/'
directory='validation/'
board_size=1000
iterations=1000

test_file="output_${board_size}x${board_size}_${iterations}_0.txt"

# Generate test file
numThreads=20
./Homework3 "$board_size" "$iterations" "$numThreads" "$directory"

numThreads=1;
./Homework5 "$board_size" "$iterations" "$numThreads" "$directory" "$test_file"
