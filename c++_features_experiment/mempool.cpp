#include <cstddef>
#include <iostream>
#include <memory>
#include <new>
#include <utility>
struct mytype {
    int x;
    mytype() : x{0} {}
    mytype(int t) : x{t} {}

    ~mytype() { x = 1; }
};

// no reclamation / compaction
template <std::size_t max_bytes> class mempool {
  public:
    mempool()
        : pool_(static_cast<std::byte *>(
              ::operator new(max_bytes, std::align_val_t(64)))),
          left_{max_bytes}, base_(pool_) {}

    template <typename T> constexpr std::size_t get_alignment() {
        return std::max<std::size_t>(64, alignof(T));
    }

    template <typename req_type, typename... Args>
    req_type *malloc(Args... args) {
        void *ptr_ = pool_;
        // align for this req_type!
        if (!std::align(get_alignment<req_type>(), sizeof(req_type), ptr_,
                        left_)) {
            throw std::bad_alloc();
        }

        // auto increments ptr_, updates left_, now aligned!
        ::new (ptr_) req_type{std::forward<req_type>(args...)};
        pool_ = static_cast<std::byte *>(ptr_) +
                sizeof(req_type);  // align next time! (easier implementation)
        left_ -= sizeof(req_type); // final left memory
        return std::launder(reinterpret_cast<req_type *>(
            ptr_)); // launder as we re-use memory (in future)
    }

    template <typename req_type> req_type *malloc() {
        void *ptr_ = pool_;
        if (!std::align(get_alignment<req_type>(), sizeof(req_type), ptr_,
                        left_)) {
            throw std::bad_alloc();
        }
        // auto increments ptr_, updates left_

        ::new (ptr_) req_type{};

        pool_ = static_cast<std::byte *>(ptr_) +
                sizeof(req_type); // align next time! (easier implementation)
        left_ -= sizeof(req_type);
        return std::launder(reinterpret_cast<req_type *>(
            ptr_)); // launder as we re-use memory (in future)
    }

    template <typename del_type> void free(del_type *ptr) { ptr->~del_type(); }

    ~mempool() { ::operator delete(base_); }

  private:
    std::byte *pool_;
    std::size_t left_; // easier to deal alignment, which auto increments ptr.
    std::byte *base_;
};

int main() {
    using std::cout;
    mempool<10000> objpool;
    cout << alignof(bool) << " " << alignof(float);
    auto *ptr = objpool.malloc<mytype>(2);
    cout << ptr->x << "\n";
    objpool.free(ptr);
    cout << ptr->x << "\n";
}