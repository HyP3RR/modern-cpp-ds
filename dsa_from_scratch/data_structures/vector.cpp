#include <iostream>
#include <new>
#include <vector>
#include "trace.h"


/*

use trace obj to track behaviour.

current:
use operator new for low level management
placement new for constructing obj in place.

future scope:
custom operator new to allocate from mem pools (custom allocator)
set_new_handler to handle bad allocs from custom allocator


things to note:
1. placement new -> call ptr.~T(), since it is not "our memory" technically
2. consistent new operator and delete operator calls! (can pick new[]  or new)
3. ensure strong exception safety unless declared noexcept!
4. ensure no resource leak of currently owned objects.

fun facts:
1. new expression may elide allocations, compiler optimises if it can find valid storage,
no allocation instruction generated

2. new expression (keyword) -> calls ::operator new to allocate bytes and constructs the object (somehwo).. we avoid it, since we would have to default construct the new memory for array.

3. operator new, supports alignment via ::operator new(sizeof(T), std::align_val_t(64));
by default it is  __STDCPP_DEFAULT_NEW_ALIGNMENT__ (16). useful if custom alignment. directly
gives bytes, no construction.

4. placement new -> T* obj = ::new(ptr) T(...) (if ptr void) or ::new(ptr) T(...)
calls ctor on memory poitner by ptr, and returns pointer T* type...
if ptr size < underlying obj -> UB
now, must manually call dtor since it's borrowed memory technically. then do operator delete.

use :: , so we always do for global scope for now.
*/



namespace prat{

  template<typename T>
  class vector{
  public:


    vector():ptr_(nullptr),size_(0),cap_(0){} 

    explicit vector(std::size_t n,const T& val):size_(n), cap_(n){
      ptr_ = reinterpret_cast<T*>(::operator new(n*sizeof(T)));
      //completely unrelated type, use reinterpret_cast... do not access any obj
      //before actually constructing it (or UB)
      for(std::size_t i = 0 ; i < n; i++){
	new(ptr_ + i) T(val);
      }
      //avoided using '=' operator for underlying! pitfall avoided.
    }
    explicit vector(std::size_t n):size_(n), cap_(n){
      ptr_ = reinterpret_cast<T*>(::operator new(n*sizeof(T))); 
      for(std::size_t i = 0 ; i < n; i++){
	new(ptr_ + i) T;
      }

      //just get X bytes and construct each one in place. never possible with new[10] etc.
    }

    void push_back(T& val){
      
      if(size_ == cap_){
	std::size_t new_cap = (cap_ > 0 ? 2*cap_ : 1); //handling for cap = 0
	re_alloc(new_cap);
      }
      new(ptr_+size_) T(val); 
      size_++;
      
     }

    void push_back(T&& val){
      if(size_ == cap_) re_alloc();
      new(ptr_+size_) T(std::move(val));
      size_++;
    }
    

    std::size_t size(){
      return size_;
    }

    T& operator[](std::size_t index){
      return ptr_[index];
    }
    
    ~vector(){
      for(std::size_t i=0;i<size_;i++){
	ptr_[i].~T();
	//we used operator new, and allocated via placement new..
	//cannot call delete directly, need to invoke destructor alag se.
      }
      ::operator delete(ptr_); //no delete[] ptr_, as we constructed bytes directly.
      //did not involve array allocation.
    }


  private:
    T* ptr_;
    std::size_t size_;
    std::size_t cap_;

    void re_alloc(std::size_t new_cap){
      //ensure exception safety
      
      T* new_ptr_ = reinterpret_cast<T*> (::operator new(new_cap*sizeof(T))); 
      for(std::size_t i=0;i<size_;i++){
	new(new_ptr_+i) T(std::move(ptr_[i]));
	//TODO: check if no-except of move, otherwise fall.. to ensure exception safety.
	//this is how standard does it.
      }
 
      std::swap(new_ptr_,ptr_);
      cap_ = new_cap;
      for(std::size_t i = 0 ; i < size_ ; i++){
	new_ptr_[i].~T(); //move does not free memory
      }
      ::operator delete(new_ptr_);
      //deletes the bytes by looking up extra book-keeping info (heap size > we req)
      
    }

    
  };


};


struct x;

int main(){
  // prat::vector<int> vec(10,2);
 
 
 prat::vector<trace>  v1;
 std::vector<trace> v2;
 
 trace obj;
 obj.x =1;
 obj.y=2;
 int z = 3;
 obj.z = &z;
 v2.push_back(obj); //v2 uses construct at
 v2.push_back(obj);
 v2.push_back(obj);
// v2.push_back(obj);
for(std::size_t i=0;i<v1.size();i++){
  std::cout <<v1[i].x <<"\n";
  v1[i].x = 3;
  std::cout <<v1[i].x <<"\n";

}


}
