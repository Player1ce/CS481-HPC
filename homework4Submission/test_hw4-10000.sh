#!/bin/bash

source /apps/profiles/modules_asax.sh.dyn

module  load  openmpi/4.1.4-gcc11
module load intel

directory='/scratch/ualclsd0201/'
#directory='validation/'

numThreads=20;
mpirun -n "$numThreads" Homework4 10000 5000 "$numThreads"
mpirun -n "$numThreads" Homework4 10000 5000 "$numThreads"
mpirun -n "$numThreads" Homework4 10000 5000 "$numThreads"
