#pragma once

#include <exe/fibers/sync/futex.hpp>
#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

// https://gobyexample.com/waitgroups

class WaitGroup {
 public:
  void Add(size_t count) {
    routines_.fetch_add(count);
  }

  void Done() {
    spin_.fetch_add(1);
    routines_.fetch_sub(1);
    routines_futex_.WakeAll();
    spin_.fetch_add(-1);
  }

  void Wait() {
    uint32_t current_routines;
    while ((current_routines = routines_.load()) != 0) {
      routines_futex_.ParkIfEqual(current_routines);
    }

    while (spin_.load() != 0) {
      wheels::SpinLockPause();
    }
  }

 private:
  twist::stdlike::atomic<uint32_t> routines_{0};
  twist::stdlike::atomic<uint32_t> spin_{0};

  FutexLike<uint32_t> routines_futex_{routines_};
};

}  // namespace exe::fibers
