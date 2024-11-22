#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(unsigned numThreads);

    template <class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};
