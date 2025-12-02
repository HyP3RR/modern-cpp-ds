#include <condition_variable>
#include <cstddef>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

using namespace std;

class threadpool {
  public:
    threadpool(size_t n) : stop{false}, threads{n} {
        for (int i = 0; i < threads; i++) {
            workers.emplace_back([this] {
                do_work();
            }); // constructs in place, reallocation using move assignment
        }
    }

    template <typename Function> void submit_job(Function &&f) {
        {
            std::lock_guard<std::mutex> lk{m};
            if (stop) {
                throw std::runtime_error("submit job after shutdown");
            }
            jobs.push(std::function<void()>(f));
        }
        start.notify_one();
    }

    ~threadpool() {
        {
            unique_lock<mutex> ulk(m);
            stop = true;
        }
        start.notify_all(); // ensure everyone exits!
        for (auto &worker : workers) {
            worker.join();
        }
    }

  private:
    bool stop;
    mutex m;
    size_t threads;
    vector<std::thread> workers;
    queue<std::function<void()>> jobs;
    condition_variable start;
    void do_work() {
        // keep working/look for work
        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> ulk(m);
                start.wait(ulk, [this] { return (stop || !jobs.empty()); });
                if (jobs.empty() && stop)
                    break;
                job = std::move(jobs.front());
                jobs.pop();
            }
            job(); // run job
        }
    }
};

int main() {}