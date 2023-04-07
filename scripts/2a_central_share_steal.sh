#!/bin/sh

#SBATCH -J task2ad       # Job name
#SBATCH -o task2ad.o%j      # Name of stdout output file
#SBATCH -e task2ad.e%j       # Name of stderr error file
#SBATCH -p skx-normal      # Queue (partition) name
#SBATCH -N 1               # Total # of nodes (must be 1 for serial)
#SBATCH -n 1               # Total # of mpi tasks (should be 1 for serial)
#SBATCH -t 01:30:00        # Run time (hh:mm:ss)

# module load intel

# module load papi/5.7.0
# papi_avail

export BUILD=$SCRATCH/task2ad

################################################################################
do_cr_share () {
    export DIMSIZE=$1
    make clean && make $BUILD/central && $BUILD/central
}

export THRESHOLD=16
export NUM_THREADS=96
for dims in 8192 4096 2048 1024 512 256 128 64
do
    do_cr_share $dims
done

################################################################################
do_dr_share () {
    export DIMSIZE=$1
    make clean && make $BUILD/share && $BUILD/share
}

export THRESHOLD=16
export NUM_THREADS=96
for dims in 8192 4096 2048 1024 512 256 128 64
do
    do_dr_share $dims
done

################################################################################
do_dr_steal () {
    export DIMSIZE=$1
    make clean && make $BUILD/steal && $BUILD/steal
}

export THRESHOLD=16
export NUM_THREADS=96
for dims in 8192 4096 2048 1024 512 256 128 64
do
    do_dr_steal $dims
done

################################################################################
do_dr_share_mod () {
    export DIMSIZE=$1
    make clean && make $BUILD/share && $BUILD/share
}

export THRESHOLD=16
export NUM_THREADS=96
export WORK_MOD=1
for dims in 8192 4096 2048 1024 512 256 128 64
do
    do_dr_share_mod $dims
done

################################################################################
do_dr_steal_mod () {
    export DIMSIZE=$1
    make clean && make $BUILD/steal && $BUILD/steal
}

export THRESHOLD=16
export NUM_THREADS=96
export WORK_MOD=1
for dims in 8192 4096 2048 1024 512 256 128 64
do
    do_dr_steal_mod $dims
done
