#ifndef SERIAL_MM_HPP
#define SERIAL_MM_HPP

#ifndef DIMSIZE
#define DIMSIZE 4
#endif

void iter_mm_ijk(float* z/*out*/, const float* x, const float* y, int size);
void iter_mm_ikj(float* z/*out*/, const float* x, const float* y, int size);

void iter_mm_jik(float* z/*out*/, const float* x, const float* y, int size);
void iter_mm_jki(float* z/*out*/, const float* x, const float* y, int size);

void iter_mm_kij(float* z/*out*/, const float* x, const float* y, int size);
void iter_mm_kji(float* z/*out*/, const float* x, const float* y, int size);

#endif
