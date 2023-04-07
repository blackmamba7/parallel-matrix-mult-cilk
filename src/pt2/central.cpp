#include <iostream>
#include <thread>
#include <vector>

#ifdef PAPI_ENABLED
#include <papi.h>
#endif

#include "central.hpp"
#include "serial_mm.hpp"
#include "utils.hpp"

#ifndef THRESHOLD
#define THRESHOLD 2
#endif

#ifndef NUM_THREADS
#define NUM_THREADS 8
#endif

void add_to_queue(const TaskInfo x) {
    std::unique_lock<std::mutex> lock(q_mutex);
    q.push_back(x);
}

void thread_helper() {
    std::unique_lock<std::mutex> lock(q_mutex);
    if (!q.empty()) {
        auto task = q.front();
        q.pop_front();
        lock.unlock();
        par_rec_mm(task.z, task.x, task.y, task.size, task.sync);
    }
}

void par_rec_mm_2(float *z, const float *x, const float *y, const int size,
                  const std::shared_ptr<SyncInfo> c_sync) {
    const float *x11 = x;
    const float *x12 = x + size / 2;
    const float *x21 = x + (DIMSIZE * size / 2);
    const float *x22 = x + (DIMSIZE * size / 2) + size / 2;
    const float *y11 = y;
    const float *y12 = y + size / 2;
    const float *y21 = y + (DIMSIZE * size / 2);
    const float *y22 = y + (DIMSIZE * size / 2) + size / 2;
    float *z11 = z;
    float *z12 = z + size / 2;
    float *z21 = z + (DIMSIZE * size / 2);
    float *z22 = z + (DIMSIZE * size / 2) + size / 2;

    add_to_queue(TaskInfo(z11, x12, y21, size / 2, c_sync));
    add_to_queue(TaskInfo(z12, x12, y22, size / 2, c_sync));
    add_to_queue(TaskInfo(z21, x22, y21, size / 2, c_sync));
    par_rec_mm(z22, x22, y22, size / 2, c_sync);
}

void par_rec_mm(float *z, const float *x, const float *y, const int size,
                const std::shared_ptr<SyncInfo> p_sync) {
    if (size <= THRESHOLD) {

        iter_mm_ikj(z, x, y, size);

        if (p_sync && p_sync->dec_and_ret()) {
            std::shared_ptr<SyncInfo> curr_sync = p_sync;
            bool curr_sync_done = curr_sync->sync_done();
            while (curr_sync) {
                if (curr_sync_done) {
                    if (curr_sync->b_add_task) {
                        TaskInfo task_info(
                            curr_sync->task_info.z, curr_sync->task_info.x,
                            curr_sync->task_info.y, curr_sync->task_info.size);
                        auto local_sync = std::make_shared<SyncInfo>(
                            task_info, curr_sync->parent);
                        local_sync->b_add_task = false;

                        par_rec_mm_2(curr_sync->task_info.z,
                                     curr_sync->task_info.x,
                                     curr_sync->task_info.y,
                                     curr_sync->task_info.size, local_sync);
                        break;
                    } else {
                        curr_sync = curr_sync->parent;
                        if (curr_sync) {
                            curr_sync_done = curr_sync->dec_and_ret();
                        } else {
                            empty = true;
                            break;
                        }
                    }
                } else {
                    break;
                }
            }
        }
    } else {
        const float *x11 = x;
        const float *x12 = x + size / 2;
        const float *x21 = x + (DIMSIZE * size / 2);
        const float *x22 = x + (DIMSIZE * size / 2) + size / 2;
        const float *y11 = y;
        const float *y12 = y + size / 2;
        const float *y21 = y + (DIMSIZE * size / 2);
        const float *y22 = y + (DIMSIZE * size / 2) + size / 2;
        float *z11 = z;
        float *z12 = z + size / 2;
        float *z21 = z + (DIMSIZE * size / 2);
        float *z22 = z + (DIMSIZE * size / 2) + size / 2;

        TaskInfo task_info(z, x, y, size);
        auto local_sync = std::make_shared<SyncInfo>(task_info, p_sync);

        add_to_queue(TaskInfo(z11, x11, y11, size / 2, local_sync));
        add_to_queue(TaskInfo(z12, x11, y12, size / 2, local_sync));
        add_to_queue(TaskInfo(z21, x21, y11, size / 2, local_sync));
        par_rec_mm(z22, x21, y12, size / 2, local_sync);
    }
}

void worker() {
    while (!empty) {
        thread_helper();
    }
}

void testpar() {
    float *x, *y, *z;

    x = new float[DIMSIZE * DIMSIZE];
    y = new float[DIMSIZE * DIMSIZE];
    generate_matrix(x);
    generate_matrix(y);

    z = new float[DIMSIZE * DIMSIZE]();

    std::vector<std::thread> threads;
    add_to_queue(TaskInfo(z, x, y, DIMSIZE, nullptr));
    for (size_t i = 0; i < NUM_THREADS - 1; i++) {
        threads.emplace_back(worker);
    }
    worker();
    for (size_t i = 0; i < NUM_THREADS - 1; i++) {
        threads[i].join();
    }

    #ifdef PRINT_AND_VERIFY_OUTPUT
    print_matrix(z);
    if(!verify_product_matrix(z, x, y)){
        throw std::logic_error("Product does not match");
    }
    #endif
    
    delete x;
    delete y;
    delete z;
}

void papi_wrap_test() {
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

    testpar();

#ifdef PAPI_ENABLED
    if(PAPI_stop(eventSet, values)!= PAPI_OK)std::cout << "ERR: " << __LINE__ << "\n";
    std::cout << DIMSIZE << ": "
              << " L1 TCM : " << values[0]
              << " L2 TCM : " << values[1]
              << " L3 TCM : " << values[2]
              <<"\n";
#endif
}

void timer_wrap_test(){
    auto start = std::chrono::steady_clock::now();

    testpar();

    auto end = std::chrono::steady_clock::now();
    auto runTimeMicroSeconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Matrix dimension: " << DIMSIZE << "\nExecution time in microseconds: "
              << runTimeMicroSeconds
              << "\n";
    
    float FLOP = 2 * pow(DIMSIZE, 3);
    float FLOPS = (FLOP * pow(10, 6)) / runTimeMicroSeconds;
    float GFLOPS = FLOPS / pow(10, 9);

    std::cout << "GFLOPS: " << GFLOPS << "\n";
}

int main() {
    // papi_wrap_test();
    timer_wrap_test();
    return 0;
}
