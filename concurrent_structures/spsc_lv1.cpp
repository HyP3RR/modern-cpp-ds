#include <iterator>
#include <mutex>
#include <condition_variable>
#include <iostream>
/*
  basic spsc thread safe queue,
  for now OS level primitives and locks allowed

wrapped around indices for spsc, limit number of objects to N-1.
use extra states, or allow increment of indices and take mask while indicing.
seperates the case of empty and filled by out = in + N or out == in..
can use full N slots now!


ensure max size power of 2, to enable binary arithmetic to take modulo
overflow possible theorectically, but not practically.


Drawbacks:
1. if writing, we block consumer reading, even if theres
no contention.
2. this approach is blocking , ie threads can be suspended indefinitely if another thread holds the lck
3. pace depends on scheduler! if scheduler takes away time slice for the locked one, no one makes progress.

soln:
1. lockfree -> atleast one thread always make progress, even if others are paused (no matter what)
2. making lockfree implementation. one thread is guaranteed to make progress. do it via atomics

 **understand the drawbacks better, lock based vs lockfree stuff

*/

template<typename T>
class SPSC{
public:
  static SPSC* get_instance(){
    static SPSC queue{};
    return &queue;
    //c++11 guarantees, thread safe initialization of static obj
    //even by multiple threads
  }

  void try_produce(T obj){
    std::lock_guard<std::mutex> lk(m_);
    if(full()) throw std::runtime_error("filled buffer");
    buffer_[mask(out_++)] = std::move(obj);
    empty_check_.notify_one();
    // if(++out_ == MAXSIZE) out_ = 0; dont wrap around!
  }
   void produce(T obj){
    std::unique_lock<std::mutex> uniq_lk(m_);
    full_check_.wait(uniq_lk, [this]{return !full();}); //wait till not full
    buffer_[mask(out_++)] = std::move(obj);
    empty_check_.notify_one();
    // if(++out_ == MAXSIZE) out_ = 0; dont wrap around!
  }

  std::unique_ptr<T> consume(){
    //wait until atleast 1 item via condition variables.
    std::unique_lock<std::mutex> uniq_lk(m_); //locked by default.
   
    empty_check_.wait(uniq_lk , [this]{return !empty(); }); //use unique lock with cv, wait to fill
    auto my_ptr = std::make_unique<T>(buffer_[mask(in_)]);
    in_++;
    full_check_.notify_one();
    //to establish exception guarantee, return pointer... copy return might throw, hence data loss.
    return my_ptr;
  }

  std::unique_ptr<T> try_consume(){
    //non blocking version
    std::lock_guard<std::mutex> lk(m_);
    if(empty())return nullptr;
    auto my_ptr = std::make_unique<T>(buffer_[mask(in_)]);
    in_++; 
    return my_ptr;
  }
  

  std::size_t mask(std::size_t index){
    //taking modulo, works MAXSIZE must be power of 2
    //lot more efficient than taking %
    return index&(MAXSIZE-1);
  }

  bool empty() {return out_ == in_;}
  std::size_t size(){return out_ - in_ ;}
  bool full() {return size() == MAXSIZE; }
  

private:
  T* buffer_;
  std::size_t in_; //consume
  std::size_t out_; //produce
  std::mutex m_;
  std::condition_variable empty_check_;
  std::condition_variable full_check_;
  static constexpr std::size_t MAXSIZE = (1<<10);

  SPSC():buffer_(new T[MAXSIZE]) , in_{0} , out_{0}{} //deletes other ctor
  ~SPSC(){
    delete[] buffer_;
  }
  
};




void add(){
  auto mq = SPSC<int>::get_instance();
  for(int i=0;i<100;i++){
    mq->produce(i);
    std::cout <<i <<" " <<1 <<"\n";
  }

}

void rem(){
  auto mq = SPSC<int>::get_instance();
  for(int i=0;i<100;i++){
    auto ptr = mq->consume();
    std::cout <<*ptr <<" " <<2 <<"\n";
  }
}


int main(){
  auto myqueue = SPSC<int>::get_instance();//get pointer to initalised queue
  std::thread t(add);
  std::thread q(rem);
  t.join();q.join();
}
