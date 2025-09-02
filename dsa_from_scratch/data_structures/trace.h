

struct  trace{
 //compiler default func written to trace func calls.
  int x,y;
  int *z;
  trace(){
    x = 1;
    y = 1;
    z = nullptr;
    std::cout <<"constructor called \n";
  }
  trace(int a, int b, int *c):  x(a), y(b) , z(c){
    std::cout <<"parameter ctor called \n";
  }
  trace(const trace& other) : x(other.x), y(other.y), z(other.z){
    std::cout <<"copy ctor called \n";
  }
  trace(trace &&other) noexcept : x(std::move(other.x)), y(std::move(other.y)),z(std::move(other.z)) {
    std::cout <<"move ctor called \n";
  }

  trace& operator=(const trace& other){
    using std::swap;
    if(this != &other){ 
    trace tmp(other);
    swap(tmp, *this);
    }
    std::cout <<"copy assignment operator called \n";
    return *this;
  }

  trace& operator=(trace&& other)noexcept{
    using std::swap;
    if(this != &other){
      z =  nullptr; //clean up (if on heap), free-ing current resource or resource leak.
      swap(other,*this);
    }
    std::cout <<"move assignment operator called \n";
    return *this;
  }
  

  
  ~trace(){
    z = nullptr;
    std::cout <<"destructor called \n";
  }
};
