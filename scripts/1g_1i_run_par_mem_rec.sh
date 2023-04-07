#!/bin/sh

#SBATCH -J par_rec_mm           # Job name
#SBATCH -o par_rec_mm.o%j       # Name of stdout output file
#SBATCH -e par_rec_mm.e%j       # Name of stderr error file
#SBATCH -p skx-normal      # Queue (partition) name
#SBATCH -N 1               # Total # of nodes (must be 1 for serial)
#SBATCH -n 1               # Total # of mpi tasks (should be 1 for serial)
#SBATCH -t 02:00:00        # Run time (hh:mm:ss)

module load intel

module load papi/5.7.0
papi_avail

export CILK_NWORKERS=96
export BUILD=$SCRATCH/par_rec_mm

do_par_rec_run_all () {
    export THRESHOLD=$1
    make clean && make $BUILD/par_rec_mm && $BUILD/par_rec_mm
}

export DIMSIZE=2048
for dims in 512 256 128 64 32 16 8 4 2 1024
do
    do_par_rec_run_all $dims
done

