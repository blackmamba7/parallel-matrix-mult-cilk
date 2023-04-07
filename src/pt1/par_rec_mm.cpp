#include <chrono>
#include <cilk/cilk.h>
#include <iostream>
#include <functional>

#ifdef PAPI_ENABLED
#include <papi.h>
#endif

#include "serial_mm.hpp"

#ifndef THRESHOLD
#define THRESHOLD 2
#endif

using par_func_t = void(float*,const float*,const float*);

void par_rec_mm(float* z/*out*/, const float* x, const float* y, int size)
{
    if (size <= THRESHOLD)
    {
        iter_mm_ikj(z, x, y, size);
    }
    else
    {
        const float* x11 = x;
        const float* x12 = x + size/2;
        const float* x21 = x + (DIMSIZE * size/2);
        const float* x22 = x + (DIMSIZE * size/2) + size/2; 
        const float* y11 = y;
        const float* y12 = y + size/2;
        const float* y21 = y + (DIMSIZE * size/2);
        const float* y22 = y + (DIMSIZE * size/2) + size/2; 
        float* z11 = z;
        float* z12 = z + size/2;
        float* z21 = z + (DIMSIZE * size/2);
        float* z22 = z + (DIMSIZE * size/2) + size/2; 

        cilk_spawn par_rec_mm(z11,x11,y11,size/2);
        cilk_spawn par_rec_mm(z12,x11,y12,size/2);
        cilk_spawn par_rec_mm(z21,x21,y11,size/2);
        par_rec_mm(z22,x21,y12,size/2);
        cilk_sync;

        cilk_spawn par_rec_mm(z11,x12,y21,size/2);
        cilk_spawn par_rec_mm(z12,x12,y22,size/2);
        cilk_spawn par_rec_mm(z21,x22,y21,size/2);
        par_rec_mm(z22,x22,y22,size/2);
        cilk_sync;
    }
}

void papi_wrap_test(float* z, const float* x, const float* y) {
#ifdef PAPI_ENABLED
    int eventSet = PAPI_NULL;
    if(PAPI_library_init(PAPI_VER_CURRENT)!=PAPI_VER_CURRENT) std::cout << "ERR: " << __LINE__ << "\n";
    if(PAPI_create_eventset(&eventSet)!= PAPI_OK)std::cout << "ERR: " << __LINE__ << "\n";
    if(PAPI_add_event(eventSet,PAPI_L1_TCM)!= PAPI_OK)std::cout << "ERR: " << __LINE__ << "\n";
    if(PAPI_add_event(eventSet,PAPI_L2_TCM)!= PAPI_OK)std::cout << "ERR: " << __LINE__ << "\n";
    if(PAPI_add_event(eventSet,PAPI_L3_TCM)!= PAPI_OK)std::cout << "ERR: " << __LINE__ << "\n";
    long long values[3];

    if(PAPI_start(eventSet)!= PAPI_OK)std::cout << "ERR: " << __LINE__ << "\n";
#endif

    par_rec_mm(z, x, y, DIMSIZE);

#ifdef PAPI_ENABLED
    if(PAPI_stop(eventSet, values)!= PAPI_OK)std::cout << "ERR: " << __LINE__ << "\n";
    std::cout << DIMSIZE << ": "
              << " L1 TCM : " << values[0]
              << " L2 TCM : " << values[1]
              << " L3 TCM : " << values[2]
              <<"\n";
#endif
}

void timer_wrap_test(float* z, const float* x, const float* y)
{

    auto start = std::chrono::steady_clock::now();

    par_rec_mm(z, x, y, DIMSIZE);

    auto end = std::chrono::steady_clock::now();
    std::cout << DIMSIZE << " time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << "\n";
}


void run_test(std::function<par_func_t> test_func)
{
    float* x = new float[DIMSIZE * DIMSIZE];
    float* y = new float[DIMSIZE * DIMSIZE];
    float* z = new float[DIMSIZE * DIMSIZE];
    test_func(z,x,y);
    delete[] x;
    delete[] y;
    delete[] z;
}

int main()
{
    run_test(papi_wrap_test);
    run_test(timer_wrap_test);
    return 0;
}
