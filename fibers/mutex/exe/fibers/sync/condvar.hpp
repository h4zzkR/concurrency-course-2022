#pragma once

#include <exe/fibers/sync/mutex.hpp>
#include <exe/fibers/sync/futex.hpp>
#include <twist/stdlike/atomic.hpp>

// std::unique_lock
#include <mutex>

namespace exe::fibers {

class CondVar {
  using Lock = std::unique_lock<Mutex>;

 public:
  void Wait(Lock& lock) {
    uint32_t val = alarm_.load();
    lock.unlock();
    alarm_futex_.ParkIfEqual(val);
    lock.lock();
  }

  void NotifyOne() {
    alarm_.fetch_add(1);
    alarm_futex_.WakeOne();
  }

  void NotifyAll() {
    alarm_.fetch_add(1);
    alarm_futex_.WakeAll();
  }

 private:
  twist::stdlike::atomic<uint32_t> alarm_{0};
  FutexLike<uint32_t> alarm_futex_{alarm_};
};

}  // namespace exe::fibers
