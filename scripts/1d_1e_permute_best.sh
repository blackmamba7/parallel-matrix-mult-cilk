#!/bin/bash

#SBATCH -J serial_permute_best           # Job name
#SBATCH -o serial_permute_best.o%j       # Name of stdout output file
#SBATCH -e serial_permute_best.e%j       # Name of stderr error file
#SBATCH -p skx-normal      # Queue (partition) name
#SBATCH -N 1               # Total # of nodes (must be 1 for serial)
#SBATCH -n 1               # Total # of mpi tasks (should be 1 for serial)
#SBATCH -t 02:30:00        # Run time (hh:mm:ss)

pwd
date

module load intel

export BUILD=$SCRATCH/serial_permute_best_e

# For correctness check enable
# export DIMSIZE=512
# export THRESHOLD==8

# do_serial_run_all () {
#     make clean && make PERMUTE_FOR="$1" $BUILD/serial_permute_best &&  ./$BUILD/serial_permute_best
# }

# perms=(
# "-DFIR=FOR -DSEC=FOR -DTHI=FOR"
# "-DFIR=FOR -DSEC=FOR -DTHI=CILK"
# "-DFIR=FOR -DSEC=CILK -DTHI=FOR"
# "-DFIR=FOR -DSEC=CILK -DTHI=CILK"
# "-DFIR=CILK -DSEC=FOR -DTHI=FOR"
# "-DFIR=CILK -DSEC=FOR -DTHI=CILK"
# "-DFIR=CILK -DSEC=CILK -DTHI=FOR"
# "-DFIR=CILK -DSEC=CILK -DTHI=CILK"
# )

# for ((i = 0; i < ${#perms[@]}; i++))
# do
#     do_serial_run_all "${perms[$i]}"
# done

 perms_valid=(
"-DFIR=FOR -DSEC=CILK -DTHI=FOR"
"-DFIR=FOR -DSEC=CILK -DTHI=CILK"
"-DFIR=CILK -DSEC=FOR -DTHI=FOR"
"-DFIR=CILK -DSEC=FOR -DTHI=CILK"
"-DFIR=FOR -DSEC=FOR -DTHI=CILK"
)   

do_threshold_run_all () {
    export DIMSIZE=1024
    echo $2
    make clean && make PERMUTE_FOR="$1" $BUILD/serial_permute_best && CILK_NWORKERS=$2 $BUILD/serial_permute_best
}

for ((i = 0; i < ${#perms_valid[@]}; i++))
do
for work in 96 64 48 32 16 8 4 2 1
do
    do_threshold_run_all "${perms_valid[$i]}" $work
done
done
