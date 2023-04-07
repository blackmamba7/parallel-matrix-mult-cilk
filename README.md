# CSE613

## Build
Input size can be specified as environment variable DIMSIZE, the minimum size of matrix to be computed serially is specified as THRESHOLD <br>
Change CXX in Makefile to change compiler <br>
`icpc` was used as the compiler for Task 1 and for PAPI tasks. `g++` was used for the other tasks<br>

To build `make` <br>
To run `./build/executable` <br>

If no build directory is specified in the env `BUILD`, then `./build` is used <br>

## Papi
Load the PAPI module on stampede or export the vars `TACC_PAPI_LIB` and `TACC_PAPI_INC` <br>
To load papi module run `module load papi`

## Experiments
All the source files are in `src` and `include`
The experiments can be reproduced with the scripts in the `scripts`<br>
