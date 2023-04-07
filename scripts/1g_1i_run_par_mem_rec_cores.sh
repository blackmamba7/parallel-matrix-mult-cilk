#!/bin/sh

#SBATCH -J par_rec_mm_cores           # Job name
#SBATCH -o par_rec_mm_cores.o%j       # Name of stdout output file
#SBATCH -e par_rec_mm_cores.e%j       # Name of stderr error file
#SBATCH -p skx-normal      # Queue (partition) name
#SBATCH -N 1               # Total # of nodes (must be 1 for serial)
#SBATCH -n 1               # Total # of mpi tasks (should be 1 for serial)
#SBATCH -t 02:00:00        # Run time (hh:mm:ss)

module load intel

module load papi/5.7.0
papi_avail

cilkn_work=96
export BUILD=$SCRATCH/par_rec_mm_cores

do_par_rec_run_all () {
    export DIMSIZE=$1
    export THRESHOLD=128
    echo $2
    make clean && make $BUILD/par_rec_mm && CILK_NWORKERS=$2 $BUILD/par_rec_mm
}

# for dims in 2048 1024 512 256 128
# do
#     do_par_rec_run_all $dims $cilkn_work
# done


for dims in 128 256 512 1024 2048
do
for work in 96 64 48 32 16 8 4 2 1
do
    do_par_rec_run_all $dims $work
done
done

