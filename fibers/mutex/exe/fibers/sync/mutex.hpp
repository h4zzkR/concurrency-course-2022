#pragma once

#include <exe/fibers/sync/futex.hpp>
#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

class Mutex {
 public:
  void Lock() {
    uint32_t expect_unlocked = 0;
    while (!is_locked_.compare_exchange_strong(expect_unlocked, 1)) {
      is_locked_ft_.ParkIfEqual(expect_unlocked);  // Futex wait
      expect_unlocked = 0;
    }
  }

  void Unlock() {
    is_locked_.store(0);
    is_locked_ft_.WakeOne();
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::stdlike::atomic<uint32_t> is_locked_{0};
  FutexLike<uint32_t> is_locked_ft_{is_locked_};
};

}  // namespace exe::fibers
