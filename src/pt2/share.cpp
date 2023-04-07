#include <iostream>
#include <thread>
#include <vector>
#include <math.h>

#ifdef PAPI_ENABLED
#include <papi.h>
#endif

#include "steal-share.hpp"
#include "serial_mm.hpp"
#include "utils.hpp"

#ifndef THRESHOLD
#define THRESHOLD 2
#endif

#define SHARE 1

#ifdef WORK_MOD
long long work[NUM_THREADS] = {0};
static std::mutex work_mutex[NUM_THREADS];

bool compareWork(int thread1, int thread2){
    bool ret;

    if(thread1 == thread2){
        return true;
    }

    if (work[thread1] <= work[thread2]){
        ret = false;
    }
    else {
        ret = true;
    }
    
    return ret;
}
#endif

int targetThreadSelect(int algo){
    //algo 1-> share || 2-> steal
    #ifdef WORK_MOD
    int cand1 = randomThread();
    int cand2 = randomThread();
    if(algo == 1){
        return compareWork(cand1, cand2) ? cand2 : cand1;
    }
    else if(algo == 2){
        return compareWork(cand1, cand2) ? cand1 : cand2;
    }
    else{
        throw std::invalid_argument("No algorithm mapping");
    }
    #endif

    #ifndef WORK_MOD
    return randomThread();
    #endif
}

void add_task_to_thread_queue(const TaskInfo x, int threadID){
    std::unique_lock<std::mutex> lock(q_mutex[threadID]);
    q[threadID].push_back(x);
    
    #ifdef WORK_MOD
    std::unique_lock<std::mutex> work_lock(work_mutex[threadID]);
    work[threadID] += pow(x.size, 3);
    work_lock.unlock();
    #endif

    lock.unlock();
}

int randomThread(){
    int randID;
    randID = rand() % NUM_THREADS;
    return randID;
}

void thread_helper(int threadID, int targetThread) {
    std::unique_lock<std::mutex> lock(q_mutex[targetThread]);
    if(!q[targetThread].empty()){

        TaskInfo task = (targetThread == threadID) ? q[targetThread].front() : q[targetThread].back();
        (targetThread == threadID) ? q[targetThread].pop_front() : q[targetThread].pop_back();

        #ifdef WORK_MOD
        std::unique_lock<std::mutex> work_lock(work_mutex[targetThread]);
        work[targetThread] -= pow(task.size, 3);
        work_lock.unlock();
        #endif

        lock.unlock();
        par_rec_mm(task.z, task.x, task.y, task.size, task.sync, threadID);
    }
}

void par_rec_mm_2(float *z, const float *x, const float *y, const int size,
                  const std::shared_ptr<SyncInfo> c_sync, int threadID) {
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

    add_task_to_thread_queue(TaskInfo(z11, x12, y21, size / 2, c_sync), targetThreadSelect(SHARE));
    add_task_to_thread_queue(TaskInfo(z12, x12, y22, size / 2, c_sync), targetThreadSelect(SHARE));
    add_task_to_thread_queue(TaskInfo(z21, x22, y21, size / 2, c_sync), targetThreadSelect(SHARE));
    par_rec_mm(z22, x22, y22, size / 2, c_sync, threadID);
}

void par_rec_mm(float *z, const float *x, const float *y, const int size,
                const std::shared_ptr<SyncInfo> p_sync, int threadID) {
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
                                     curr_sync->task_info.size, local_sync, threadID);
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

        add_task_to_thread_queue(TaskInfo(z11, x11, y11, size / 2, local_sync), targetThreadSelect(SHARE));
        add_task_to_thread_queue(TaskInfo(z12, x11, y12, size / 2, local_sync), targetThreadSelect(SHARE));
        add_task_to_thread_queue(TaskInfo(z21, x21, y11, size / 2, local_sync), targetThreadSelect(SHARE));
        par_rec_mm(z22, x21, y12, size / 2, local_sync, threadID);
    }
}

void worker(int threadID){
    int randID = threadID;

    while(!empty){
        if(q[threadID].size() != 0){
            thread_helper(threadID, threadID);
        }
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
    add_task_to_thread_queue(TaskInfo(z, x, y, DIMSIZE, nullptr), 0);
    for (size_t i = 0; i < NUM_THREADS - 1; i++) {
        threads.emplace_back(worker, i);
    }
    worker(NUM_THREADS-1);
    for (size_t i = 0; i < NUM_THREADS - 1; i++) {
        threads[i].join();
    }

    #ifdef PRINT_AND_VERIFY_OUTPUT
    print_matrix(z);
    if(!verify_product_matrix(z,x,y)){
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
