#include <deque>
#include <mutex>

#ifndef NUM_THREADS
#define NUM_THREADS 8
#endif

/* std::mutex s_mutex; */
bool empty = false;

class SyncInfo;

class TaskInfo {
  public:
    TaskInfo(float *z_, const float *x_, const float *y_, const int size_,
             const std::shared_ptr<SyncInfo> sync_ = nullptr)
        : z(z_), x(x_), y(y_), size(size_), sync(sync_){};

    TaskInfo(const TaskInfo &ti_)
        : z(ti_.z), x(ti_.x), y(ti_.y), size(ti_.size), sync(ti_.sync){};

  public:
    float *z;
    const float *x;
    const float *y;
    const int size;
    const std::shared_ptr<SyncInfo> sync;
};

class SyncInfo {
  public:
    SyncInfo(const TaskInfo task_info_,
             const std::shared_ptr<SyncInfo> parent_ = nullptr)
        : task_info(task_info_), parent(parent_), i_count(4),
          b_add_task(true){};

    SyncInfo(const SyncInfo &sync_info)
        : task_info(sync_info.task_info), parent(sync_info.parent), i_count(4),
          b_add_task(true){};

    bool dec_and_ret() {
        std::unique_lock<std::mutex> lock(s_mutex);
        i_count--;
        if (i_count < 0)
        {
            throw std::runtime_error("ERR");
        }
        b_sync_done = i_count == 0 ? true : false;
        return b_sync_done;
    }

    bool sync_done() {
        std::unique_lock<std::mutex> lock(s_mutex);
        b_sync_done = i_count <= 0 ? true : false;
        return b_sync_done;
    }

    const TaskInfo task_info;
    const std::shared_ptr<SyncInfo> parent = nullptr;
    int i_count;
    bool b_add_task;
    bool b_sync_done;
    std::mutex s_mutex;
};

static std::mutex q_mutex[NUM_THREADS];
std::deque<TaskInfo> q[NUM_THREADS];

bool compareWork(int thread1, int thread2);
int targetThreadSelect(int algo);
void add_task_to_thread_queue(const TaskInfo x, int threadID);
int randomThread();
void thread_helper(int threadID, int targetThread);
void worker(int threadID);
void testpar();
void par_rec_mm(float *z, const float *x, const float *y, const int size,
                const std::shared_ptr<SyncInfo> p_sync, int threadID);
void par_rec_mm_2(float *z, const float *x, float *y, const int size,
                  const std::shared_ptr<SyncInfo> c_sync, int threadID);
