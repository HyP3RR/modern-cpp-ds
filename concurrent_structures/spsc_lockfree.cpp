#include <chrono>
#include <iostream>
#include <atomic>
#include <thread>
#include <new>
/*

lock free SPSC,
ROB is guaranteed to commit instructions in order!
but it is to mainly preserve the appearence of in-order execution
to the rest of the pipeline. But, the actual memory update is done after
by storing inside STORE BUFFER. this buffer might commit instructions
out of order (to optimise memory bandwidth/usage.)
so it can happen push_count is incremented but the new data copied
is not visible to other threads.
Store buffer is the culprit which interacts to L1 cache/memory etc.

so even if cache coherence is there, this doesn't guarantee thread safety
because of store buffer reordering.

here by using acquire release semantics, we can atleast guarantee that all
operations before it are committed to memory, when the release is synchronised
with acquire... hence strong ordering.

**aquire release, seq consistent promise ordering ONLY for atomic operations...
global shared non-atomic variables can still cause problems. we can use fences
for these cases.

by default all operations on atomic, including reads are seq consistent.
*/

/*
  we use the same modulo logic and monotonic head and tail variables.

 to prevent false sharing leading to cache invalidations and inc memory
bandwidth,
we align the head and tail to hardware destructive interference size (ie atleast
that much gap) this resulted in 4x performance for this...

to further improve, we note head, tail only increase. if we cache the tail_
variable, we use it till the buffer becomes empty, and then re-load the tail_
atomic, and recheck our conditions. this limits the time we use acquire
ordering for pop. same can be doine for pushing and head_ variable.
*/






template<typename T>
class SPSC{
public:
  SPSC() : capacity_(1024), ring_(reinterpret_cast<T*>(::operator new(sizeof(T)*capacity_))), head_{}, tail_{}{
  }

  bool push(const T& val){
    // tail only incremented by push thread since SPSC, so no need to load it by
    // seq cst...
    // local variable usage is preferred to calculate sizes etc, lesser memory cycles wasted... 
    auto tail = tail_.load(std::memory_order_relaxed); //use local variables directly by loading
    auto head =  head_.load(std::memory_order_acquire);
    if(tail - head != capacity_){
      new (ring_ + tail%capacity_) T(val); //invoke copy constructor via placement new.
      tail_.store(1 + tail, std::memory_order_release);
      return true;
    }
    else return false;    
  }

  bool  pop(T& place){
    // to ensure exception safety, user allocates memory to place popped object
    // in  directly via move.
    auto head = head_.load(std::memory_order_relaxed);
    auto tail = tail_.load(std::memory_order_acquire);
    if(head < tail ){
      new (&place) T(std::move(ring_[head%capacity_])); //no allocation, only move construction
      //via placement new.
      ring_[head%capacity_].~T(); //destroy the obj to prevent memory leak.
      head_.store(1 + head, std::memory_order_release);
      return true;
    }
    else return false;

  }



  ~SPSC(){
    while(!empty()){
      ring_[head_%capacity_].~T(); //placement new usage should be combined with destructor calling
      head_++;
    }
    ::operator delete(ring_); //since we did operator new, didnt make array via new.    
  }

private:
  std::size_t capacity_;
  T*  ring_;
  alignas(std::hardware_destructive_interference_size) std::atomic<size_t> head_; //pop at head
  alignas(std::hardware_destructive_interference_size) std::atomic<
      size_t> tail_; // push at tail , align to prevent sharing of cache line...
//this hardware constant is a compile time constant.  
  static_assert(
      std::atomic<std::size_t>::is_always_lock_free); // must be true, compile
                                                      // time check.

  bool full() {
    return (tail_.load(std::memory_order_acquire) - head_.load(std::memory_order_acquire) == capacity_);
}
bool empty() {
    return tail_.load(std::memory_order_acquire) == head_.load(std::memory_order_acquire);
}
  

};




int main() {

}
