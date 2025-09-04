# modern-cpp-ds

As a C++ enthusiast, this repository contains implementations of algorithms and data structures I find interesting, written using **modern C++ (C++20 and beyond)**.  
The goal is to explore practical coding patterns, experiment with system-level behavior, and revisit classic algorithmic ideas in the context of today’s compilers and hardware.

---
## 🚀 Features & Highlights

- **Modern C++ Practices**: Utilizing C++20+ features to write basic data structures from scratch.
- **System-Level Experiments**: Custom memory allocators, stack regions experimentations.
- **Algorithmic Exploration**: Classic data structures and algorithms reimplemented in a modern context from scratch for clarity and efficiency.

---

## 📌 Upcoming Ideas & Experiments

1. **Stack Region via `mmap`/`munmap`**  
   - Each thread’s stack region is created via `mmap`.  
   - Simulate arena allocation and investigate how virtual address collisions are prevented.  
   - Verify behavior using `strace` on a multithreaded program.

2. **Template Metaprogramming Bubble Sort Revisited**  
   - Inspired by [Todd Veldhuizen’s article](http://www.cs.rpi.edu/~musser/design/blitz/meta-art.html).  
   - In the 1990s, TMP-based bubble sort was reported to be **~1.6× faster** than the classic implementation due to compiler limitations.  
   - Hypothesis: with modern compilers (loop unrolling, inlining, vectorization), the TMP approach may no longer outperform loops — it might even be slower.  
   - A **re-vamped performance comparison** is implemented in `tmp_bubblesort`.

---

## 🛠️ Requirements
- C++20 (or later) compliant compiler (e.g. GCC, Clang, MSVC).
- CMake (optional, for builds).
- Linux recommended (for `mmap`/`munmap` experiments).

---
