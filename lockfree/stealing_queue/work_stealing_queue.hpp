#pragma once

#include <array>
#include <span>
#include <twist/stdlike/atomic.hpp>
#include <wheels/logging/logging.hpp>

namespace lockfree {

// TODO: understand SyncOrder completely

// Ok, I think, I got memory models. Simple observation from Lipovsky
// seminar - relaxed is just bare read/write instruction with no guaranties.

// Single-Producer / Multi-Consumer Bounded Ring Buffer
// Circular array based lock-free queue
// Key features: doesn't need to allocate anything on the heap
template <typename T, size_t Capacity>
class WorkStealingQueue {
  struct Slot {
    // UPDATE: Master said, it is good, but with some mm magic.
    twist::stdlike::atomic<T*> item{nullptr};
  };

  // ABA prevention
  // https://stackoverflow.com/questions/33333363/built-in-mod-vs-custom-mod-function-improve-the-performance-of-modulus-op
  uint32_t BoundedPosition(uint32_t pos) {
    return pos >= k_capacity_ ? pos % k_capacity_ : pos;
  }

 public:
  // SINGLE thread usage
  bool TryPush(T* item) {
    // everything is und3r COИTЯOL

    uint32_t current_free_slot =
        next_free_slot_.load(std::memory_order_relaxed);
    uint32_t current_head = head_slot_.load();

    if (BoundedPosition(current_free_slot + 1) ==
        BoundedPosition(current_head)) {
      return false;
    }

    auto bounded_pos = BoundedPosition(current_free_slot);

    // there is no situation for one atomic with two ambiguous writes (stores)
    buffer_[bounded_pos].item.store(item, std::memory_order_release);
    next_free_slot_.fetch_add(1, std::memory_order_release);  // protect upper
    return true;
  }

  // Returns nullptr if queue is empty
  T* TryPop() {
    Slot* captured;
    uint32_t head = head_slot_.load(std::memory_order_acquire);

    // Guaranties: SW
    do {
      // Here we go if someone moved head index, so captured value is invalid.
      // Retake it.

      if (BoundedPosition(next_free_slot_.load(std::memory_order_acquire)) ==
          BoundedPosition(head)) {
        return nullptr;
      }
      captured = &buffer_[BoundedPosition(head)];
      sched_yield();  // ?
    } while (!head_slot_.compare_exchange_strong(head, head + 1));

    // read/read access - bare read
    return captured->item.load(std::memory_order_relaxed);
  }

  // Returns number of tasks
  size_t Grab(std::span<T*> out_buffer) {
    auto span_size = out_buffer.size();
    uint32_t head = head_slot_.load(std::memory_order_acquire);
    size_t grabbed_cnt;

    // We start copy tasks naively
    // And if sequence of slots was affected by other threads
    // Then we start again
    // And loop until sequence weren't changed.

    do {
      auto next_tail = next_free_slot_.load(std::memory_order_acquire);
      grabbed_cnt = std::min((size_t)(next_tail - head), span_size);

      auto iter = out_buffer.begin();
      for (size_t i = 0; i < grabbed_cnt; ++i) {
        auto pos = BoundedPosition(head + i);

        // there is no situation for one atomic with two ambiguous writes
        *(iter++) = buffer_[pos].item.load(std::memory_order_acquire);
      }
    } while (!head_slot_.compare_exchange_strong(head, head + grabbed_cnt));

    return grabbed_cnt;
  }

 private:
  const size_t k_capacity_ = Capacity + 1;
  std::array<Slot, Capacity + 1> buffer_;  // sentinel for fullness check

  twist::stdlike::atomic<uint32_t> next_free_slot_{0};  // push
  twist::stdlike::atomic<uint32_t> head_slot_{0};       // pop
};

}  // namespace lockfree
