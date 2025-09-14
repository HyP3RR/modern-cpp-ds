#include <iostream>
#include <atomic>
#include <thread>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

/*
  implement my own version of mutex,
  try adding better primitives like fairness, bounded wait etc.
  internal mutexes offer fairness guarantees.

 internally, modern mutex are implemented like this:
 if mutex is taken, the kernel puts the running process
 on waitqueue. the scheduler cannot run processes on wait queues, on releasing
 mutex, OS picks a process to wake up (should be ideally fair among threads).
 linux offers futex also, which always ensures theres no syscall, if the underlying
 lock is actually empty.... this is done via compare exchange, where checking for
 free-ness is implemented in userspace directly.


textbook MUTEX -> generic mututal exclusion mechanism
programming MUTEX -> OS offered primitive for mutual exclusion



i cannot make the OS offered mutex, but mutual exclusion can stil be achieved
by spinlocks, user space locks etc.
**lock based atomics internally use OS primitives like futex to implement the instruction.

 */



//my cpp20 attempt.
namespace prat{


  class mutex{
  public:
    mutex():lk(ATOMIC_FLAG_INIT){}

    void lock() noexcept {
      //while prev value is true, keep in loop
      while(lk.test_and_set(std::memory_order_acquire)){
	//this thread yield -> reduces cpu wastage
	lk.wait(true, std::memory_order_relaxed); //cpp20 onwards offers to wait until find any change from true, done via notify.
	//by default atomic flag always lock free, just the blocking Part is OS implemented.
      }
    }

    void unlock(){
      lk.clear(std::memory_order_release); //set to false.
      lk.notify_one(); //cpp20 onwards -> not fair.
    }


  private:
    std::atomic_flag lk;
    
  };
};
//no spinning, less contention, NO fairness.
//technically stupid to use .wait, as it calls futex internally... have to spin otherwise, anyway.
//so officially, without using .wait, it is a spinlock.


//spinlock with fairness
namespace fairlock{
  //while locking, each thread gets ticket
  //while unlocking, the counter is inc, so the oldest
  //calling thread gets the access.
  class mutex{
  public:
    mutex() = default; //by default 0
    
    void lock() noexcept {
      int my_turn = ticket.fetch_add(1);
      while(my_turn != turn.load()){}
    }

    void unlock(){
      turn.fetch_add(1);
    }
  private:
    std::atomic<int> ticket;
    std::atomic<int> turn;
  };


};


namespace optimised_spinlock{
  //doing repeteadly .exchange true, takes exclusive state
  //access to cache, modifying and invaliding lock's copies of other cores
  //instead, request shared state, wait till false then contend for locking.
  //this reduces cache ping pong and memory can use it's resources in better place.
  class mutex{
  public:
    mutex():lk_(false){}

    void lock(){
      while(lk_.exchange(true)){
	while(lk_.load()){
	  std::this_thread::yield(); //give up time slice.
	}
      }
    }

  private:
    std::atomic<bool> lk_;
  };
};


//default mutexes use futex internally.
//custom implement a futex, using futex syscall.. puts process on waitqueue
//to optimise direct kernel calling, spin for a while incase lock frees early.-> glibc library does this.
//this is adaptive futex, may spin and then backoff.
namespace futex_lock{

  //static for internal linkage.
  static inline int futex_wait(uint32_t* addr, int val ){
    return syscall(SYS_futex, addr, FUTEX_WAIT_PRIVATE, val, NULL , NULL , 0);
    //SYS_futx, address, option, value, timeout_ptr,multi_addr_opn , extra_val 
  }

};




int main(){
  prat::mutex m;

}
