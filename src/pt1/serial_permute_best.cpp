#include <chrono>
#include <functional>

#include "utils.hpp"

#include <cilk/cilk.h>

#ifdef PAPI_ENABLED
#include <papi.h>
#endif

#define FOR(x) for(int x = 0; x < size; x++)
#define CILK(x) cilk_for(int x = 0; x < size; x++)

#define UPDATE z[j + DIMSIZE * i] += x[k + DIMSIZE * i] * y[j + DIMSIZE * k];


using iter_func_t = void(float*,const float*,const float*,const int);
void iter_mm_ikj_mod(float* z/*out*/, const float* x, const float* y, int size) {
    FIR(i)
        SEC(k)
            THI(j)
                UPDATE
}

void iter_mm_kij_mod(float* z/*out*/, const float* x, const float* y, int size) {
    FIR(k)
        SEC(i)
            THI(j)
                UPDATE
}


void timer_wrap_test(std::function<iter_func_t> iter_func,
                  float* z, const float* x, const float* y, std::string id = "") {

    auto start = std::chrono::steady_clock::now();
    iter_func(z,x,y, DIMSIZE);
    auto end = std::chrono::steady_clock::now();
    std::cout << __func__ << " " << id << " : "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << "\n";
    /* verify_product_matrix(z, x, y); */
}

void papi_wrap_test(std::function<iter_func_t> iter_func,
                  float* z, const float* x, const float* y, std::string id = "") {

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
    iter_func(z,x,y, DIMSIZE);
#ifdef PAPI_ENABLED
    if(PAPI_stop(eventSet, values)!= PAPI_OK)std::cout << "ERR: " << __LINE__ << "\n";
    std::cout << __func__ << " " << id << " "
              << " L1 TCM : " << values[0]
              << " L2 TCM : " << values[1]
              << " L3 TCM : " << values[2]
              <<"\n";
#endif

}

void run_all()
{
    float* x = new float[DIMSIZE * DIMSIZE];
    float* y = new float[DIMSIZE * DIMSIZE];
    float* z = new float[DIMSIZE * DIMSIZE];

    generate_matrix(x);
    generate_matrix(y);
    zero_matrix(z);

    timer_wrap_test(iter_mm_ikj_mod, z,x,y, std::string("ikj:") + std::to_string(DIMSIZE));
    timer_wrap_test(iter_mm_ikj_mod, z,x,y, std::string("ikj:") + std::to_string(DIMSIZE));
    zero_matrix(z);
    papi_wrap_test(iter_mm_ikj_mod, z,x,y, std::string("ikj:") + std::to_string(DIMSIZE));

    delete[] x;
    delete[] y;
    delete[] z;
}

int main() {
    run_all();
    return 0;
}
