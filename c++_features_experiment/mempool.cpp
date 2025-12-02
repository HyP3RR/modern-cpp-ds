

#include <bits/stdc++.h>

using namespace std;
template <typename T> class Pool {
    union chunk {
        alignas(T) std::byte buf[sizeof(T)];
        chunk *next;
    };

    chunk *free_list = nullptr;
    chunk *blocks = nullptr;

  public:
    Pool(size_t n) {
        blocks = new chunk[n]();
        free_list = &blocks[0];
        for (int i = 0; i < n - 1; i++) {
            blocks[i].next = &blocks[i + 1];
        }
        blocks[n - 1].next = nullptr;
    }
    T *allocate() {
        chunk *c;
        if (free_list) {
            c = free_list;
            free_list = free_list->next;
        } else {
            throw std::bad_alloc();
        }

        return new (c->buf) T(); // placement new into buf
    }

    void deallocate(T *obj) {
        obj->~T();                                 // destroy the object
        chunk *c = reinterpret_cast<chunk *>(obj); // go back to chunk
        c->next = free_list;                       // push back to freelist
        free_list = c;
    }

    ~Pool() {
        // assuming user gives out all the used struct
        delete[] blocks;
    }
};

int main() {}