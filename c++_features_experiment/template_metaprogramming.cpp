#include <iostream>
#include <cassert>





//template metaprogramming based bubble sort
template<int i, int j>
constexpr void inbubsort(int *data){



  
  if constexpr (j<i){
    if(data[j]>data[j+1]){
      int temp = data[j];
      data[j] = data[j+1];
      data[j+1] = temp;
    }
  inbubsort<i,j+1>(data);
  }
  else inbubsort<i-1,0>(data);
}

template<>
constexpr void inbubsort<0,0>(int *data){}


//safe cast, executes at runtime
template<typename to, typename from>
to safe_cast(from obj){

  
  char hello[sizeof(to) <= sizeof(obj)? 1: -1];
  //nneed compile time checking to give error messages
  return reinterpret_cast<to>(obj);
}



//std::is_same TMP based implementation
template<typename T, typename V>
struct is_same{
  static constexpr bool value = false;
};

template<typename T>
struct is_same<T,T>{
  static constexpr bool value = true;
};


template<typename identify_type>
struct scott_struct;


//temp
template<bool> struct compileerror;

template<> struct compileerror<true>;

int main(){
  int arr[5] = {4,5,1,3,2};
  inbubsort<5,0>(arr);
  for(int i=0;i<5;i++)std::cout <<arr[i]<<" ";

  std::cout <<"\n";
  double*  x;
  auto obj = safe_cast<double*>(x);
  std::cout <<obj <<"\n";

  std::cout <<is_same<int,int>::value <<"\n";
}
