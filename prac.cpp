#include <bits/stdc++.h>

using namespace std;

// template <typename T> class mempool {
//   public:
//     mempool(size_t n) {
//         buf = static_cast<T *>(
//             ::operator new(sizeof(T) * n, std::align_val_t(alignof(T))));
//             //startt at alignof(T), and each multiple n*sizeof(T)
//         for (int i = 0; i < n; i++) {
//             void *current = buf + i * (sizeof(T));
//             void *next = nullptr;
//             if (i < n - 1)
//                 next = buf + (i + 1) * (sizeof(T));
//             *reinterpret_cast<void **>(current) = next;
//         }
//         front = buf;
//     }
//     ~mempool() { ::operator delete(buf, std::align_val_t(alignof(T))); }

//     T *allocate() {
//         if (!front)
//             throw bad_alloc();
//         T *freeblock = std::launder(reinterpret_cast<T *>(front));
//         front = *reinterpret_cast<void **>(front);
//         return freeblock;
//     }

//     void deallocate(T *ptr) {
//         ptr->~T();
//         *reinterpret_cast<void **>(ptr) = front;
//         front = ptr;
//     }

//   private:
//     alignas(alignof(T)) T *buf;
//     void *front;
// };
// // sizeof(T) > 8 bybtes

// take care of alignment, thread safety

// another via vector<char*> to block size and poolsize
// popback and return back approach etc...

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

template <typename T> void alloc() {
    alignas(alignof(T)) byte obj[sizeof(T)];
    new (obj) T();
    T *x = reinterpret_cast<T *>(obj);
}

int func() { return 2; }

int (*fu)() = &func;

template <typename T> struct scott;

void *memmove(void *dest, const void *src, size_t count) {
    if (!src || !dest)
        return nullptr;
    const std::byte *ptr = reinterpret_cast<const std::byte *>(src);
    std::byte *dst = reinterpret_cast<std::byte *>(dest);
    if (dst < ptr) {
        // from start to end
        const std::byte *end = ptr + count;
        while (ptr != end) {
            *dst = *ptr;
            ptr++;
            dst++;
            // ensured simd?
        }
    } else if (dst > ptr) {
        ptr += count;
        dst += count;
        while (count--) {
            *--dst = *--ptr;
            // ensure simd? give hint for not aliasing!
        }
    }
    return dest;
}

template <int N> constexpr int v = N * v<N - 1>;
template <> constexpr int v<1> = 1;

template <typename T> struct bruh : T {};

int main() {

    // auto lam = []() -> int { return 1; };
    // vector<bool> a(100);
    // variant<int, bool, string> v;
    // v = "helo";
    // cout << get<string>(v) << "\n";
    // std::visit(
    //     [](auto &&obj) {
    //         scott<decltype(obj)> t;
    //     },
    //     v);

    // int x = 5;
    // int &ref = x;
    // int &&rref = 7;
    // scott<decltype(rref)> a0;
    // scott<decltype(std::move(x))> a1;
    // scott<decltype((rref))> a2;
    // scott<decltype((std::move(x)))> a3;
    // int a[8];
    // cout << sizeof(a) << "\n";
    // int x = 1, y = 2;
    // vector<std::reference_wrapper<int>> a = {x, y};

    // cout << a[0].get() << "\n";
    // int z = 12;
    // a[0] = ref(z);
    // int &t = a[0]; // converting contructor to T&
    // std::any ttt = "hi";
    // cout << ttt.type().name() << '\n';
    // unique_ptr<int[]> p = make_unique<int[]>(21);
}