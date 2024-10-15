#!/bin/bash

#!/bin/bash

#module load intel

#directory='/scratch/ualclsd0201/'
directory='output/'
test_file='output_5x5_10_0.txt'

numThreads=1;
./Homework3 5 10 "$numThreads" "$directory" "$test_file"


numThreads=2;
./Homework3 5 10 "$numThreads" "$directory" "$test_file"


numThreads=3;
./Homework3 5 10 "$numThreads" "$directory" "$test_file"


numThreads=4;
./Homework3 5 10 "$numThreads" "$directory" "$test_file"


numThreads=5;
./Homework3 5 10 "$numThreads" "$directory" "$test_file"

for fileNum in {0..4}
do
  diff -s "$directory"output_5x5_10_"$fileNum".txt "$directory$test_file"
done
