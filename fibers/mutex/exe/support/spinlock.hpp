#pragma once

#include <wheels/support/cpu.hpp>
#include <twist/stdlike/atomic.hpp>

#include <wheels/logging/logging.hpp>

namespace exe::support {

// Test-and-TAS spinlock

// We need SW for sync critical sections through atomics
// SO is not needed - atomic is only one.

class SpinLock {
 public:
  void Lock() {
    // acquire bcz main idea is to see changes from unlock (?)
    while (locked_.exchange(true, std::memory_order_acquire)) {
      while (locked_.load(std::memory_order_acquire)) {
        wheels::SpinLockPause();
      }
    }
  }

  void Unlock() {
    locked_.store(false, std::memory_order_release);
  }

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::stdlike::atomic<bool> locked_{false};
};

}  // namespace exe::support
