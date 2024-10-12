#!/bin/bash

module load intel

numThreads=1;

./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"

numThreads=2;

./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"

numThreads=4;

./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"

numThreads=8;

./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"

numThreads=10;

./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"

numThreads=16;

./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"

numThreads=20;

./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"
./Homework3 5000 5000 "$numThreads"