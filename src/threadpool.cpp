
#include "threadpool.h"

ThreadPool::ThreadPool(unsigned num_threads) {
    stop = false;
    active_tasks = 0;
    for (unsigned i = 0; i < num_threads; ++i) {
        workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                { //lock block
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    //wait for stop condition
                    condition.wait(lock, [this]() { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) return;
                    task = std::move(tasks.front());
                    tasks.pop();
                    active_tasks++;
                }
                task();
                { //lock block
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    active_tasks--;
                    if (active_tasks == 0 && tasks.empty()) {
                        done_condition.notify_all(); //notify all when tasks are done
                    }
                }
            }
            });
    }
}

ThreadPool::~ThreadPool() {
    { //lock block
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }

    condition.notify_all();
    for (std::thread& worker : workers) {
        worker.join();
    }
}

void ThreadPool::WaitUntilFinished() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    done_condition.wait(lock, [this]() { return tasks.empty() && active_tasks == 0; });
}
