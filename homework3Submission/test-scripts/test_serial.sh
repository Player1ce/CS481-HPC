#!/bin/bash

module load intel

numThreads=1;
directory='/scratch/ualclsd0201/'
test_file='output_5000x5000_5000_0.txt'

./../Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./../Homework3 5000 5000 "$numThreads" "$directory" "$test_file"
./../Homework3 5000 5000 "$numThreads" "$directory" "$test_file"

for fileNum in {1..3}
do
  diff "$directory"output_5000x5000_5000_"$fileNum".txt "$directory""$test_file"
done