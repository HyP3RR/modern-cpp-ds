#include <vector>
#include <set>



template <typename T> struct scott;


int main() {
  std::vector<double> v;
  auto it = v.begin();
  scott<decltype(it)> t;
}
