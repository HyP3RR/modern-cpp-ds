#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <iostream>
#include <queue>
#include <vector>

template<typename T>
class thread_safe_queue {
public:
  explicit thread_safe_queue() = default;
      // disable other ctors
      // technically not needed, as compiler only generates default ctors
    //if the underlying data is copyable etc.      

  thread_safe_queue(const thread_safe_queue<T> &other) = delete;
  thread_safe_queue(thread_safe_queue<T> &&other) noexcept = delete;

  thread_safe_queue<T> &operator=(const thread_safe_queue<T> &other) = delete;
  thread_safe_queue<T>& operator=(thread_safe_queue<T> &&other) = delete;


  template <typename U> void add_elem(U &&obj) {
    //forwarding reference added, to accept r and l values directly.    
    std::lock_guard<std::mutex> lk_guard(mut);
    safe_queue.push(std::forward<U>(obj));
    empty_check.notify_one();
  }

  T  wait_and_pop() {
    // combine front and pop, prevent the invalid state issue.
    // i thought of returning shared_ptr via make_shared , but doing a heap
    // allocations while removing is an unnecessary cost.
    //hence return simply T.. with strong exception safety guarantee, as we pop later    
    std::unique_lock<std::mutex> lk_unique(mut);
    empty_check.wait(lk_unique, [this] { return !safe_queue.empty(); });
    T obj = std::move(
        safe_queue.front()); // can mess up , if move ctor throws.
                             // but most standard types offer noexcept guarantee
    safe_queue.pop(); //safe to pop now, to offer exception safety.
    return obj;
  }

  bool empty() const {
    // fun-fact: locking mutex here is incorrect -> as we marked func const.
    // use mutable, to allow mutex to mutate/change even in these condn.    
    std::lock_guard<std::mutex> lk_guard(mut);
    return (!safe_queue.empty());
    }
  
private:
  std::queue<T> safe_queue;
  std::condition_variable empty_check;
  mutable std::mutex mut;
};


void func_add(thread_safe_queue<int> &obj) {
  int i = rand();
  std::cout <<"add" <<i <<"\n";
  obj.add_elem(i);
}

void func_rem(thread_safe_queue<int> &obj) {
  int v = obj.wait_and_pop();
  std::cout <<"pop" <<v <<"\n";  
}

int main() {
  std::vector<std::thread> threads;
  threads.reserve(10);
  thread_safe_queue<int> obj;
  for (int i = 0; i < 10; i+=2) {
    threads.emplace_back(func_add, std::ref(obj));
    threads.emplace_back(func_rem, std::ref(obj));
  }
  for (auto &t : threads) {
    if(t.joinable())t.join();
  }
  
  
  
  return 0;  
}

