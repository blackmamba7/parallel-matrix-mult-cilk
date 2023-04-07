#include <chrono>
#include <functional>

#include "serial_mm.hpp"
#include "utils.hpp"

#ifdef PAPI_ENABLED
#include <papi.h>
#endif

using iter_func_t = void(float*,const float*,const float*,const int);

void timer_wrap_test(std::function<iter_func_t> iter_func,
                  float* z, const float* x, const float* y, std::string id = "") {

    auto start = std::chrono::steady_clock::now();
    iter_func(z,x,y, DIMSIZE);
    auto end = std::chrono::steady_clock::now();
    std::cout << __func__ << " " << id << " : "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()
              << "\n";
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

    timer_wrap_test(iter_mm_ijk, z,x,y, std::string("ijk:") + std::to_string(DIMSIZE));
    timer_wrap_test(iter_mm_ikj, z,x,y, std::string("ikj:") + std::to_string(DIMSIZE));
    timer_wrap_test(iter_mm_jik, z,x,y, std::string("jik:") + std::to_string(DIMSIZE));
    timer_wrap_test(iter_mm_jki, z,x,y, std::string("jki:") + std::to_string(DIMSIZE));
    timer_wrap_test(iter_mm_kij, z,x,y, std::string("kij:") + std::to_string(DIMSIZE));
    timer_wrap_test(iter_mm_kji, z,x,y, std::string("kji:") + std::to_string(DIMSIZE));

    papi_wrap_test(iter_mm_ijk, z,x,y, std::string("ijk:") + std::to_string(DIMSIZE));
    papi_wrap_test(iter_mm_ikj, z,x,y, std::string("ikj:") + std::to_string(DIMSIZE));
    papi_wrap_test(iter_mm_jik, z,x,y, std::string("jik:") + std::to_string(DIMSIZE));
    papi_wrap_test(iter_mm_jki, z,x,y, std::string("jki:") + std::to_string(DIMSIZE));
    papi_wrap_test(iter_mm_kij, z,x,y, std::string("kij:") + std::to_string(DIMSIZE));
    papi_wrap_test(iter_mm_kji, z,x,y, std::string("kji:") + std::to_string(DIMSIZE));

    delete[] x;
    delete[] y;
    delete[] z;
}

int main() {
    run_all();
    return 0;
}
