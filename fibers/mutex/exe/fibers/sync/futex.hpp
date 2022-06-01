#pragma once

#include <exe/fibers/core/api.hpp>
#include <twist/stdlike/atomic.hpp>

#include <wheels/intrusive/list.hpp>
#include <wheels/support/assert.hpp>

#include <exe/support/spinlock.hpp>
#include <exe/fibers/core/awaiter.hpp>

#include <exe/fibers/core/fiber.hpp>
#include <wheels/logging/logging.hpp>

namespace exe::fibers {

template <typename T>
class FutexLike {
 public:
  explicit FutexLike(twist::stdlike::atomic<T>& cell) : futex_val_(cell) {
  }

  ~FutexLike() {
    //    WakeAll();
    WHEELS_ASSERT(queue_.IsEmpty(), "World is broken, kill yourself");
    // Check that wait queue is empty
  }

  // Park current fiber if cell.load() == `old`
  void ParkIfEqual(T old) {
    spinner_.Lock();  // fucking bug was here

    if (futex_val_.load() != old) {
      spinner_.Unlock();
      return;
    }

    auto aw = FutexAwaiter(Fiber::Self(), spinner_);
    queue_.PushBack(&aw);
    self::Suspend(&aw);
  }

  void WakeOne() {
    spinner_.Lock();
    if (!queue_.IsEmpty()) {
      queue_.PopFront()->handler.Schedule();
      //      queue_.PopFront()->handler.Resume();
    }
    spinner_.Unlock();
  }

  void WakeAll() {
    spinner_.Lock();
    while (!queue_.IsEmpty()) {  // get all, not one in cycle TODO
      queue_.PopFront()->handler.Schedule();
    }
    spinner_.Unlock();
  }

 private:
  wheels::IntrusiveList<FutexAwaiter> queue_;
  twist::stdlike::atomic<T>& futex_val_;
  support::SpinLock spinner_;
};

}  // namespace exe::fibers
