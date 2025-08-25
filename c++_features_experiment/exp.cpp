#include <future>
#include <thread>
#include <iostream>



void f(int x, std::promise<int> p) {

  p.set_value(x*x);
  
}

class A {};

class B : public A {};

void func(A &b){}

int main() {
  std::promise<int> p;
  std::future<int> fut = p.get_future();

  std::thread t(f, 10, std::move(p));
  std::cout << fut.get() << "\n";
  std::promise<int> g;  
  fut = g.get_future();
  t.join();
  B obj;
  func(obj);  
}
