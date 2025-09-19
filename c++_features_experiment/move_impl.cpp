#include <iostream>



template <typename T> struct remove_reference {
  using type = T;
  };

  template <typename T> struct remove_reference<T &> {
    using type = T; //partial specialization
  };

  template <typename T>
  struct remove_reference<T &&> {
    using type = T;
  };

  template <typename T>
  constexpr typename remove_reference<T>::type&& my_mov(T &&obj) {
    return static_cast<typename remove_reference<T>::type &&>(obj);
    // dependent templates are assumed to be values, not types, need typename
    // decltype(T) can be int&... so to remove lvalue references , we needed
    // that struct, for direct use static_cast<decltype(obj)&&> works...
    // unlesss, we have defined int &obj = 5... then decltype(obj) is int&... so
    // it'll mess up, so names with references wont be allowed usually    
    }



  

int main() {



  
}
