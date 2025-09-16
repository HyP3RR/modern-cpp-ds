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
directly via this pointer!! (no penalty size of class).. empty class  contains no non-static data mem (directly/indirectly)

4. how to enable EBO with no size overhead + introduce obj for func_ptr etc!
stateless lambdas seem to not incur any size penalty!  function pointers do!

nice observations
1.  stateless lambdas (0 size penalty , due to EBO by closure class)
2. func_ptr (8B size penalty, extra pointer overhead)
3. std::function (32B size penalty! extra meta data overhead!)

idea -> for stateless simple do nothing, we inherit from deleter, which works
for both lambdas and default_delete as they are struct/closure class type.
that's why using default deleter as struct type helps.

solve deleter while incorporating ebo, must include T* ptr inside the compressed object,
otherwise adding an object of deleter is not considered EBO.. 
thus, a compressed obj of deleter and pointer , template specialised on basis of
empty lambda (ie is_empty), accordingly define it
we utilise ebo in the compressed object, since we don't define an obj of base member
default_delete! while it is not valid if we do T* ptr and define base obj of compressed_deleter
in unique pointer. (completely wrong design)

*/
  

  
  template<typename T>
  struct default_delete{
    void operator()(T* ptr){ delete ptr;} //default deletion via EBO.
  };
  
  template<typename T, typename deleter , bool empty = std::is_empty<deleter>::value>
  struct compressed_obj : private deleter{ //inherit from stateless lambda or default_del
    T* ptr_;
    //dont declare obj of base, hence ebo valid.
    compressed_obj() = default;
    compressed_obj(T* obj) :  ptr_(obj) {}
    compressed_obj(T* obj, deleter d): deleter(d), ptr_(obj){} //to conform to lambdas, no memory alloted, base initialised first.
    compressed_obj(compressed_obj<T, deleter> &&other) noexcept
        : ptr_(other.ptr_) {
      other.ptr_ = nullptr;
      }

      compressed_obj<T, deleter> &
      operator=(compressed_obj<T, deleter> &&other) {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
	return *this;        
        }

        void my_delete() { this->operator()(ptr_); }
   void call_deleter(T* ptr){ this->operator()(ptr_);}        
  };

  template<typename T, typename deleter>
  struct compressed_obj<T,deleter,false>{
    T* ptr_;
    deleter custom_deleter_; //need it in this non-ebo case.

    compressed_obj() = default;
    compressed_obj(T* obj, deleter d) : ptr_(obj), custom_deleter_(d) {}
    compressed_obj (compressed_obj<T, deleter> && other) noexcept : ptr_(other.ptr_), custom_deleter_(other.custom_deleter_) {
      other.ptr_ = nullptr;
      }

     compressed_obj<T, deleter> &
      operator=(compressed_obj<T, deleter> &&other) {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        custom_deleter_ = other.custom_deleter; // dont delete the custom
                                                // deleter pointer of original.
	return *this;        
        }  
    void my_delete(){custom_deleter_(ptr_);}
    void call_deleter(T* ptr){custom_deleter_(ptr);}
  };

  /*
    honestly, if youre in cpp20, [[no_unique_address]]
    takes care of this, so no need to hack ebo.
    template<typename T, typename deleter>
    struct compressed_obj : private deleter{
    T* ptr_;
    [[no_unique_address]] deleter general_deleter;

    compressed_obj() = default;
    compressed_obj(T* obj, deleter d) : ptr_(obj), general_deleter(d) {}
    
    void my_delete(){general_deleter(ptr_);}
};
   */

  
  //default deleter and inheritence to enable EBO.
  template<typename T, typename deleter = default_delete<T>> 
  class unique_ptr{
  public:
    
    unique_ptr(T* obj = nullptr): obj_{obj}{} //T deduced when declaring class, T* thus compulsory for ptr type.
    unique_ptr(T* obj,  deleter func) : obj_{obj,func} {}

    unique_ptr(const unique_ptr<T,deleter>& other) = delete;
    unique_ptr(unique_ptr<T,deleter>&& other) noexcept : obj_(std::move(other.obj_)) {}

    unique_ptr<T,deleter>& operator= (const unique_ptr<T,deleter>& other) = delete;
    unique_ptr<T, deleter> &operator=(unique_ptr<T, deleter> &&other) noexcept {
      if (this != &other) {
	reset(); //free allocations
        obj_ = std::move(other.obj_);
        }
      return *this;      
    }

    T* get() const noexcept{
      return obj_.ptr_;
      }
    
    T* release() noexcept{
      // return pointer only releasing ownership, without deleting
      T *tmp = get();
      obj_.ptr_ = nullptr;
      return tmp;
      }
      void reset(T *p = nullptr) noexcept {
        T *old = obj_.ptr_;
        if(old == p) return;
        obj_.ptr_ = p; 
        if(old) call_deleter(old); //delete later to avoid leaking old, but need specialised overload to accept ptr
      }

    T* operator->() const {return obj_.ptr_;}
    T& operator*() const {
	return *obj_.ptr_;
        }
    
    ~unique_ptr() noexcept {
      if (obj_.ptr_)
        obj_.my_delete();
      //deallocates the ptr too.
    }
  private:
    compressed_obj<T,deleter> obj_;
  };
};


template<typename T>
struct scott{};

auto del = [](int* ptr){delete ptr;};

void func(int *ptr) { delete ptr; }


int main(){
 // prat::unique_ptr<int, decltype(del)> ptr(new int,del); //calling delete on stack alloted ptr is UB
 // std::unique_ptr<int, decltype(del)> g(new int, del);
  //std::unique_ptr<int, void(*)(int*)> ptr2(new int,func);  //decltype(func) not a func ptr.

  //std::cout <<sizeof(prat::unique_ptr<int>) << " " <<sizeof(std::unique_ptr<int>) <<"\n";
  //std::cout <<sizeof(ptr) <<" " <<sizeof(g)  <<"\n";
  //std::cout <<sizeof(prat::unique_ptr<int,void(*)(int*)>) << " " <<sizeof(std::unique_ptr<int,void(*)(int*)>) <<"\n";
  //std::cout <<sizeof(prat::unique_ptr<std::function<void(int*)>>) << " " <<sizeof(std::unique_ptr<int,std::function<void(int*)>>) <<"\n";
  using namespace std;
  std::unique_ptr<int> a(make_unique<int>(5));
  cout << (*a) << "\n";
  int *ptr = a.get();
  cout << *ptr << "\n";
  unique_ptr<int> t = std::move(a);
  cout << (*t) << "\n";
  //cout <<t->

  t.reset();
  // cout <<t <<"\n";
  prat::unique_ptr<int> p(new int(5));
  std::cout <<*p <<"\n";
  }

