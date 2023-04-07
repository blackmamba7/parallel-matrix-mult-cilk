#include "serial_mm.hpp"

#define FOR(x) for(int x = 0; x < size; x++)
#define UPDATE z[j + DIMSIZE * i] += x[k + DIMSIZE * i] * y[j + DIMSIZE * k];



void iter_mm_ijk(float* z/*out*/, const float* x, const float* y, int size)
{
    FOR(i)
        FOR(j)
            FOR(k)
                UPDATE
}

void iter_mm_ikj(float* z/*out*/, const float* x, const float* y, int size){
    FOR(i)
        FOR(k)
            FOR(j)
                UPDATE
}

void iter_mm_jik(float* z/*out*/, const float* x, const float* y, int size)
{
    FOR(j)
        FOR(i)
            FOR(k)
                UPDATE
}

void iter_mm_jki(float* z/*out*/, const float* x, const float* y, int size)
{

    FOR(j)
        FOR(k)
            FOR(i)
                UPDATE
}

void iter_mm_kij(float* z/*out*/, const float* x, const float* y, int size)
{
    FOR(k)
        FOR(i)
            FOR(j)
                UPDATE
}

void iter_mm_kji(float* z/*out*/, const float* x, const float* y, int size)
{
    FOR(k)
        FOR(j)
            FOR(i)
                UPDATE
}
