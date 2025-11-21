// MPMCQueue.hpp
// Minimal, auditable Vyukov MPMC bounded queue.
// - Lock-free (no mutexes).
// - Bounded fixed capacity (power of two).
// - try_enqueue / try_dequeue semantics (non-blocking).
// - Use only atomics with clear acquire/release ordering.
//
// ============================================================================
// ALGORITHM OVERVIEW
// ============================================================================
// Each cell has a "sequence" number that encodes its state:
//   - seq == pos        → slot is EMPTY, ready for producer at position `pos`
//   - seq == pos + 1    → slot is FULL, ready for consumer at position `pos`
//
// Producers and consumers claim positions via CAS on enqueue_pos_/dequeue_pos_.
// The sequence number acts as a lock-free "turn" indicator.
//
// Lifecycle of slot at index i (capacity = 4, mask = 3):
//   Initial:     seq = i (e.g., seq = 2 for slot 2)
//   After enq:   seq = pos + 1 (signals "data ready")
//   After deq:   seq = pos + mask + 1 = pos + 4 (signals "slot free for next
//   cycle")
//
// This means slot 2 cycles through sequence values: 2 → 3 → 6 → 7 → 10 → ...
// ============================================================================

#ifndef MPMCQUEUE_HPP
#define MPMCQUEUE_HPP

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <typename T> class mpmc_bounded_queue {
  public:
    mpmc_bounded_queue(std::size_t buffer_size)
        : buffer_(new cell_t[buffer_size]), buffer_mask_(buffer_size - 1) {
        // buffer_size must be power of 2 for fast modulo via bitwise AND
        assert((buffer_size >= 2) && ((buffer_size & (buffer_size - 1)) == 0));
        // Initialize each slot's sequence to its index (slot i is ready for
        // producer at pos i)
        for (std::size_t i = 0; i != buffer_size; i += 1)
            buffer_[i].sequence_.store(i, std::memory_order_relaxed);
        enqueue_pos_.store(0, std::memory_order_relaxed);
        dequeue_pos_.store(0, std::memory_order_relaxed);
    }

    ~mpmc_bounded_queue() { delete[] buffer_; }

    bool enqueue(T const &data) {
        cell_t *cell;
        std::size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_]; // pos % capacity
            std::size_t seq = cell->sequence_.load(std::memory_order_acquire);

            // Compare sequence to expected value for an empty slot
            // dif == 0: slot is free for this producer position
            // dif <  0: slot still occupied (queue full)
            // dif >  0: another producer took this pos, reload and retry
            std::intptr_t dif = static_cast<std::intptr_t>(seq) -
                                static_cast<std::intptr_t>(pos);
            if (dif == 0) {
                // Try to claim this position; CAS ensures only one producer
                // wins
                if (enqueue_pos_.compare_exchange_weak(
                        pos, pos + 1, std::memory_order_relaxed))
                    break; // Success! We own this slot now
                // CAS failed: another producer claimed pos, loop will retry
                // with updated pos
            } else if (dif < 0)
                return false; // Queue is full
            else
                pos = enqueue_pos_.load(
                    std::memory_order_relaxed); // Reload current position
        }

        // === CRITICAL SECTION (single producer owns this cell) ===
        cell->data_ = data;
        // Publish: seq = pos + 1 tells consumers "data is ready"
        // Release ordering ensures data_ write is visible before seq update
        cell->sequence_.store(pos + 1, std::memory_order_release);

        return true;
    }

    bool dequeue(T &data) {
        cell_t *cell;
        std::size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
        for (;;) {
            cell = &buffer_[pos & buffer_mask_]; // pos % capacity
            std::size_t seq = cell->sequence_.load(std::memory_order_acquire);

            // For dequeue, we check seq == pos + 1 (slot has data)
            // dif == 0: slot has data for this consumer position
            // dif <  0: producer hasn't written yet (queue empty)
            // dif >  0: another consumer took this pos, reload and retry
            std::intptr_t dif = static_cast<std::intptr_t>(seq) -
                                static_cast<std::intptr_t>(pos + 1);
            if (dif == 0) {
                // Try to claim this position
                if (dequeue_pos_.compare_exchange_weak(
                        pos, pos + 1, std::memory_order_relaxed))
                    break; // Success! We own this slot now
            } else if (dif < 0)
                return false; // Queue is empty
            else
                pos = dequeue_pos_.load(std::memory_order_relaxed);
        }

        // === CRITICAL SECTION (single consumer owns this cell) ===
        data = cell->data_;
        // Publish: seq = pos + buffer_mask_ + 1 = pos + capacity
        // This marks the slot as free for the NEXT cycle's producer
        // (producer at position pos + capacity will see seq == their pos)
        cell->sequence_.store(pos + buffer_mask_ + 1,
                              std::memory_order_release);

        return true;
    }
    mpmc_bounded_queue(mpmc_bounded_queue const &) = delete;
    void operator=(mpmc_bounded_queue const &) = delete;

  private:
    struct cell_t {
        std::atomic<std::size_t>
            sequence_; // Encodes slot state (empty/full + cycle)
        T data_;       // Storage for the queued item
    };

    cell_t *const buffer_;          // Contiguous array of cells
    std::size_t const buffer_mask_; // capacity - 1, for fast modulo

    // === Cache line padding to prevent false sharing ===
    // enqueue_pos_ and dequeue_pos_ are hot variables written by different
    // threads. Placing them on separate cache lines avoids performance-killing
    // cache ping-pong.
    alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> enqueue_pos_; // Next position for producers
    alignas(std::hardware_destructive_interference_size)
        std::atomic<std::size_t> dequeue_pos_; // Next position for consumers
};

#endif // MPMCQUEUE_HPP

int main() { mpmc_bounded_queue<double> queue{2048}; }