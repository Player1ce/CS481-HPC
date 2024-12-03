#!/bin/bash

source /apps/profiles/modules_asax.sh.dyn

module load cuda

directory='/scratch/ualclsd0201/'
#directory='validation/'
board_size=5000
iterations=5000
test_file="output_${board_size}x${board_size}_${iterations}_0.txt"

numThreads=1;
./Homework5 "$board_size" "$iterations" "$numThreads" "$directory" "$test_file"
./Homework5 "$board_size" "$iterations" "$numThreads" "$directory" "$test_file"
./Homework5 "$board_size" "$iterations" "$numThreads" "$directory" "$test_file"

test_file="output_${board_size}x${board_size}_${iterations}_0.txt"

board_size=10000
iterations=5000
test_file="output_${board_size}x${board_size}_${iterations}_0.txt"

numThreads=1;
./Homework5 "$board_size" "$iterations" "$numThreads" "$directory" "$test_file"
./Homework5 "$board_size" "$iterations" "$numThreads" "$directory" "$test_file"
./Homework5 "$board_size" "$iterations" "$numThreads" "$directory" "$test_file"
