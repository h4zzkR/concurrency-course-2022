#pragma once

#include <deque>
#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <wheels/intrusive/forward_list.hpp>

#include <exe/executors/execute.hpp>
#include <exe/executors/executor.hpp>

namespace exe::support {

// Unbounded blocking multi-producers/multi-consumers queue
using exe::executors::TaskBase;

// template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(TaskBase* value) {
    if (!put_allowed_.load()) {
      return false;
    }

    std::lock_guard guard(mutex_);
    buffer_.PushBack(value);

    queue_nempty_cv_.notify_one();
    return true;
  }

  TaskBase* Take() {
    std::unique_lock guard(mutex_);

    while (true) {
      if (buffer_.IsEmpty()) {
        if (!put_allowed_.load()) {
          return nullptr;
        }
        queue_nempty_cv_.wait(guard);
      } else {
        break;
      }
    }

    auto val = buffer_.PopFront();
    return val;
  }

  void Close() {
    CloseImpl(/*clear=*/false);
  }

  void Cancel() {
    CloseImpl(/*clear=*/true);
  }

 private:
  void CloseImpl(bool clear) {
    std::lock_guard guard(mutex_);
    put_allowed_.store(false);
    if (clear) {
      while (!buffer_.IsEmpty()) {
        buffer_.PopFront()->Discard();
      }
    }
    queue_nempty_cv_.notify_all();
  }

  bool TryTake() {
    std::unique_lock guard(mutex_);
    return buffer_.IsEmpty();
  }

 private:
  wheels::IntrusiveForwardList<TaskBase> buffer_;

  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable queue_nempty_cv_;
  twist::stdlike::atomic<bool> put_allowed_{true};
};

}  // namespace exe::support
