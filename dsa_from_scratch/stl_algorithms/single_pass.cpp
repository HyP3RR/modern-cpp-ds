#include <iostream>
#include <vector>


namespace prat{
  /*
    we follow option 1, for now, directly use std::iterator_traits, then include our own support to implement
    stl algo's.

   linear  search algos: (input iter min)
  1. iter  find(iter begin, iter end, v val)
  2. iter find_if(iter begin, iter end, condition_lambda/func)
  3. iter::difference_type  std::count(iter first, iter end, const T& val)


   forward search / subsequence based: (forward it minimum)
   1. std::adjacent_find
   2. std::search etc.

   sorted/binsearch based (min forward, random access iter is optmised)
   1. std::lower_bound
   2. std::upper_bound
   3. std::equal_range
   4. std::binary_search

  also output iterator / contiguous iterator  ones, but not discussed yet

  must-use cpp20 , function template  argument deduction directly via params.
   */


  template<typename iter,typename T>
  iter find_impl(iter begin, iter end, T&& value, std::input_iterator_tag){
    while(begin != end){
      if(value == (*begin))return begin;
      begin++;  
    }
    return end;
  }
  
template<typename iter, typename T>
iter find(iter begin, iter end, T&& value){
  //use universal ref, as rvalues can be there also
  
    //since lowest is input iter, we dont need to overload the template,
    //we use the principle, the hierarchy struct, derived class can bind to base,
    //but never vice versa!
    return find_impl(begin, end, value, typename std::iterator_traits<iter>::iterator_category{});
    //although, category looks like a function, it is default constructing a temporary!
    //better to use {} , as scott meyers says () can cause uninitialised fundamental types.

}
  
  

};

int main(){
  std::vector<int> vec(10);
  for(int i=0;i<10;i++)vec[i] = i;
  std::cout <<prat::find(vec.begin(),vec.end(),8)-vec.begin()<<"\n";
  //works!
}
