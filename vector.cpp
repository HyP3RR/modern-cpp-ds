#include <iostream>
#include<vector>

struct foo {
    int x;
  foo() {}
  //    foo( const foo & ) noexcept { std::cout << "copy\n"; }
  //    foo( foo && ) noexcept { std::cout << "move\n"; }
  foo( const foo & )  { std::cout << "copy\n"; }
  foo( foo && )  { std::cout << "move\n"; }

  ~foo() noexcept {}
};

namespace prat{

    template<typename T> //no allocator
    class vector{
        //in reality, allocator holds the private heap array. we doont have it so we need to do it. by basics.
        struct vector_impl{
            // size_t sz;
            T* start_;
            T* end_;
            T* cap_; //pointer to cap end +1

            vector_impl() noexcept: start_(), end_(start_), cap_(start_) {
            }
            vector_impl(size_t _n): start_(new T[_n]), end_(start_ + _n),cap_(start_+_n){ //defining size moves end_ ptr.
            }
            vector_impl& operator=(vector_impl& obj){
                //copy
                std::cout <<"Hi, I was copied \n";
                if(this != &obj){
                    //free resources
                    delete[] start_;
                    size_t size = obj.cap_- obj.start_;
                    start_ = new T[size];
                    std::copy(obj.start_,obj.end_,start_);
                    end_ = start_  + (obj.end_ -obj.start_);
                    cap_ = start_ + size;
                }
                return *this;
            }
            vector_impl& operator=(vector_impl&& obj) noexcept {
                std::cout <<"Hi, I was moved \n";
                if(&obj != this){
                delete[] start_;
                start_ = obj.start_;
                end_ = obj.end_;
                cap_  = obj.cap_;
                obj.end_ = obj.start_ = obj.cap_ =  nullptr;
                }
                return *this;          
            }
            ~vector_impl(){
                delete[] start_;
            }
        };
        vector_impl _M_impl;

        public:
        explicit vector(): _M_impl(){}
        explicit vector(size_t n) : _M_impl(n){}
        size_t size(){
            return _M_impl.end_ - _M_impl.start_;
        }
        size_t capacity(){
            return _M_impl.cap_ - _M_impl.start_;
        }
        void push_back(T val) {
            if(_M_impl.cap_ == _M_impl.end_){
                //reallocate
                size_t new_size =  (_M_impl.end_-_M_impl.start_)<<1 == 0? 1: (_M_impl.end_-_M_impl.start_)<<1 ;
                vector_impl tmp_(new_size);//new array
                auto new_it = tmp_.start_;
                for(auto it = _M_impl.start_ ; it != _M_impl.end_; it++){
                    *new_it = std::move(*(it));
                    new_it++;
                }
                tmp_.end_ = new_it; //move back the end pointer where the end of cur data lies.
                _M_impl = std::move(tmp_);
            }
           *_M_impl.end_ = val;
            _M_impl.end_++;
            
        }

        decltype(auto) operator[](size_t index){
            return _M_impl.start_[index];
            //returns T&, auto takes T& removes & and matches... so decltype(auto) is best, gives exact type taken by auto.
        }

        ~vector()=default;
    
    };


}


int main(){
    // prat::vector<int> a;
    // std::cout <<a.size() <<" " <<a.capacity() <<"\n";
    // a.push_back(1);
    // std::cout <<a.size() <<" " <<a.capacity() <<"\n";
    // a.push_back(1);
    // std::cout <<a.size() <<" " <<a.capacity() <<"\n";
    // a.push_back(1);
    // std::cout <<a.size() <<" " <<a.capacity() <<"\n";
    // a.push_back(1);
    // std::cout <<a.size() <<" " <<a.capacity() <<"\n";
    // a.push_back(1);
    // std::cout <<a.size() <<" " <<a.capacity() <<"\n";
    // a[3] = 10;
    // for(int i=0;i<a.size();i++){
    //     std::cout <<a[i] <<" ";
    // }
    // std::cout <<"\n";
    prat::vector<foo> v;
    std::vector<foo> v2;
    foo obj;
    obj.x = 1;
    for(int i=0;i<3;i++){
        v2.push_back(obj);
    }
    //one diff between my vec and std::vec,
    //my internal push_back, uses the = operator for the internal obj. (to copy/move the items)
    //while the actual stl vector, it doesn't use the internal object's = operator.
    //figure it out.
    //also check how is the behaviour when memcpy cannot be used for internal obj,
    //if std::vec utilises the copy/move assignment op for custom defined object.
}