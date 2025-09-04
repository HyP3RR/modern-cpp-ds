#include "trace.h"
#include <memory>
#include <functional>

namespace prat{

  /*
    problems faced:

2.  
template<typename other_T, typename other_deleter>
unique_ptr(const unique_ptr<other_T,other_deleter>& other) = delete;
does not work!! deleting this does not disable other copy ctor!


3. creating default deleter, and storing it's object incurs extra bytes penalty as even
empty classes have ->0 size..
can use EBO (empty base optimisation and just inherit from it, and call the default deleter!)
directly via this pointer!! (no penalty size of class)

4. how to enable EBO with no size overhead + introduce obj for func_ptr etc!
stateless lambdas seem to not incur any size penalty!  function pointers do!

nice observations
1.  stateless lambdas (0 size penalty , due to EBO by closure class)
2. func_ptr (8B size penalty, extra pointer overhead)
3. std::function (32B size penalty! extra meta data overhead!)

idea -> for stateless simple do nothing, we inherit from deleter, which works
for both lambdas and default_delete as they are struct/closure class type.
i hope it's intuitive now why 2nd parameter is never a function pointer type,
but a struct type! 

*/
  

  
  template<typename T>
  struct default_delete{
    void operator()(T* ptr){ delete ptr;} //default deletion via EBO.
  };
  
  template<typename T>
  struct custom_delete{
    
  };
 
  
  //default deleter and inheritence to enable EBO.
  template<typename T, typename deleter = default_delete<T>> 
  class unique_ptr : private deleter{
  public:
    unique_ptr():ptr_(nullptr),deleter(){
      
    } 
    unique_ptr(T* obj):ptr_(obj),deleter(){} //T deduced when declaring class, T* thus compulsory for ptr type.
    unique_ptr(T* obj,  deleter& func) : ptr_(obj), deleter(&func){}

    unique_ptr(const unique_ptr<T,deleter>& other) = delete;
    unique_ptr(unique_ptr<T,deleter>&& other){
      
    }

    unique_ptr<T,deleter>& operator= (const unique_ptr<T,deleter>& other) = delete;
    unique_ptr<T,deleter>& operator= (unique_ptr<T,deleter> &&other){

    }


    ~unique_ptr(){
      this->operator()(ptr_); //invoke deletion operator via EBO! (no obj create)
      //if deleter is type lambda, then we inherit from its closure class also! so size remains 8 (if stateless)!
    }
  private:
    T* ptr_;
  };
};


template<typename T>
struct scott{};

auto del = [](int* ptr){delete ptr;};

void func(int *ptr){delete ptr;}

int main(){
  std::unique_ptr<int, decltype(del)> ptr(new int,del); //calling delete on stack alloted ptr is UB
  std::unique_ptr<int, decltype(del)> g;
  std::unique_ptr<int, void(*)(int*)> ptr2(new int,func);  //decltype(func) not a func ptr.
  g = std::move(ptr);
  //prat::unique_ptr<int,decltype(del)> p(new int,del);
  //prat::unique_ptr<double> q(p);
 std::cout <<sizeof(ptr2) <<"\n";
 std::cout <<sizeof(std::unique_ptr<int,std::function<void(int*)>>) <<" " <<sizeof(prat::unique_ptr<int,decltype(del)>) <<"\n";
  
 
  
  
}

