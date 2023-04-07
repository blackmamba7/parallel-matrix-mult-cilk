#!/bin/bash

#SBATCH -J serial_run_all           # Job name
#SBATCH -o serial_run_all.o%j       # Name of stdout output file
#SBATCH -e serial_run_all.e%j       # Name of stderr error file
#SBATCH -p skx-normal      # Queue (partition) name
#SBATCH -N 1               # Total # of nodes (must be 1 for serial)
#SBATCH -n 1               # Total # of mpi tasks (should be 1 for serial)
#SBATCH -t 02:00:00        # Run time (hh:mm:ss)

pwd
date

module load intel

module load papi/5.7.0
papi_avail

export BUILD=$SCRATCH/serial_run_all

do_serial_run_all () {
    export DIMSIZE=$1
    make clean && make $BUILD/serial_run_all && $BUILD/serial_run_all
}

for dims in 2 4 8 16 32 64 128 256 512 1024 2048 4096
do
    do_serial_run_all $dims
done

