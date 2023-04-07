#!/bin/sh

#SBATCH -J task2cd       # Job name
#SBATCH -o task2cd.o%j      # Name of stdout output file
#SBATCH -e task2cd.e%j       # Name of stderr error file
#SBATCH -p skx-normal      # Queue (partition) name
#SBATCH -N 1               # Total # of nodes (must be 1 for serial)
#SBATCH -n 1               # Total # of mpi tasks (should be 1 for serial)
#SBATCH -t 01:30:00        # Run time (hh:mm:ss)

# module load intel

# module load papi/5.7.0
# papi_avail

export BUILD=$SCRATCH/task2cd

################################################################################
do_cr_share () {
    export NUM_THREADS=$1
    make clean && make $BUILD/central && $BUILD/central
}

export THRESHOLD=128
export DIMSIZE=4096
for threads in 2 4 8 16 32 64 96
do
    do_cr_share $threads
done

################################################################################
do_dr_share () {
    export NUM_THREADS=$1
    make clean && make $BUILD/share && $BUILD/share
}

export THRESHOLD=128
export DIMSIZE=4096
for threads in 2 4 8 16 32 64 96
do
    do_dr_share $threads
done

################################################################################
do_dr_steal () {
    export NUM_THREADS=$1
    make clean && make $BUILD/steal && $BUILD/steal
}

export THRESHOLD=128
export DIMSIZE=4096
for threads in 2 4 8 16 32 64 96
do
    do_dr_steal $threads
done

################################################################################
do_dr_share_mod () {
    export NUM_THREADS=$1
    make clean && make $BUILD/share && $BUILD/share
}

export THRESHOLD=128
export DIMSIZE=4096
export WORK_MOD=1
for threads in 2 4 8 16 32 64 96
do
    do_dr_share_mod $threads
done

################################################################################
do_dr_steal_mod () {
    export NUM_THREADS=$1
    make clean && make $BUILD/steal && $BUILD/steal
}

export THRESHOLD=128
export DIMSIZE=4096
export WORK_MOD=1
for threads in 2 4 8 16 32 64 96
do
    do_dr_steal_mod $threads
done
