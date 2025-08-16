#include <chrono>
#include <thread>
#include <iostream>


using namespace std;

void print() {
  
  cout <<"hello hi there" <<"\n";
}



// custom implementation of jthreads
// work more like guarded threads tho
//RAII of std::thread
class jthreads {
  std::thread guarded_thread;

public:
  explicit jthreads(std::thread t) : guarded_thread(std::move(t)) {
    if (!guarded_thread.joinable()) {
      throw "logic erro";
      }
  }
  jthreads(const jthreads &other) = delete;
  jthreads &operator=(const jthreads &other) = delete;
  //copy obv not defined for this.  


  ~jthreads() {
    if(guarded_thread.joinable())guarded_thread.join();
      }
};

// to add -> stop tokens and stop source to match jthreads
// directly pass func ptr etc to jthread obj and construct thread from there.


int main() {

  jthreads obj{std::thread(print)};
  
      
   std::cout <<std::thread::hardware_concurrency() <<"\n";   
  
  
  
}
