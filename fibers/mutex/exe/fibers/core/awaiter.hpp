#pragma once

#include <exe/fibers/core/handle.hpp>
#include <wheels/intrusive/list.hpp>
#include <exe/support/spinlock.hpp>

namespace exe::fibers {

struct IAwaiter {
  virtual void operator()() = 0;
  virtual ~IAwaiter() = default;
};

struct ReScheduleAwaiter : IAwaiter {
  explicit ReScheduleAwaiter(FiberHandle fiber) : handler(fiber) {
  }

  void operator()() override {
    handler.Schedule();
  }

  FiberHandle handler;
};

struct FutexAwaiter : IAwaiter, wheels::IntrusiveListNode<FutexAwaiter> {
  explicit FutexAwaiter(FiberHandle handle, support::SpinLock& lock)
      : handler(handle), lock(lock) {
  }

  void operator()() override {
    lock.Unlock();
  }

  FiberHandle handler;
  support::SpinLock& lock;
};

struct DoNothingAwaiter : IAwaiter {
  DoNothingAwaiter() {
  }
  void operator()() override {
  }
};

}  // namespace exe::fibers
