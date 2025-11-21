#include <cstddef>





template<std::size_t SIZE>
class mempool {
public:
  mempool() : pool(static_cast<std::byte*>(::operator new(MAX_BYTES))) {}

  // call int* x = mempool::alloc<int*>()
  

  
private:
  std::byte* pool;  
  static constexpr std::size_t MAX_BYTES = SIZE;


  
};
