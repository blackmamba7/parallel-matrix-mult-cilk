#ifndef _INCLUDE_UTILS_
#define _INCLUDE_UTILS_
#include <iostream>
#include <random>
#include <cmath>
#include "serial_mm.hpp"

#define LO -10000
#define HI 10000

void generate_matrix(float* x) {
    for (int i = 0; i < DIMSIZE; i++) {
        for (int j = 0; j < DIMSIZE; j++) {
             x[j + DIMSIZE * i] = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
        }
    }
}

void zero_matrix(float* x) {
    for (int i = 0; i < DIMSIZE; i++) {
        for (int j = 0; j < DIMSIZE; j++) {
             x[j + DIMSIZE * i] = 0.0f;
        }
    }
}

void print_matrix(float * x) {
    for (int i = 0; i < DIMSIZE; i++) {
        for (int j = 0; j < DIMSIZE; j++) {
            std::cout << x[j + DIMSIZE * i] << " ";
        }
        std::cout << "\n";
    }
}

bool verify_product_matrix(float *z, float *x, float *y){
    bool ret = true;
    float* z_serial;
    z_serial = new float[DIMSIZE * DIMSIZE]();
    iter_mm_ikj(z_serial, x, y, DIMSIZE);
    
    std::cout<<"\nSerial calculation - \n";
    print_matrix(z_serial);
	
    for(int i=0; i<DIMSIZE * DIMSIZE; i++){
        if(z_serial[i] != z[i]){
           std::cout << "ERROR!!!!!!!\n";
           ret = false; 
           break;
        }
    }
    
    delete z_serial;
    return ret;
}

#endif // _INCLUDE_UTILS_
