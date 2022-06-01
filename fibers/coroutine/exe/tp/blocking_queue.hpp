#pragma once

#include <deque>
#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <optional>

namespace exe::tp {

// Unbounded blocking multi-producers/multi-consumers queue

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T value) {
    if (!put_allowed_.load()) {
      return false;
    }

    std::lock_guard guard(mutex_);
    buffer_.push_back(std::forward<T>(value));
    queue_nempty_cv_.notify_one();
    return true;
  }

  std::optional<T> Take() {
    std::unique_lock guard(mutex_);

    while (true) {
      if (buffer_.empty()) {
        if (!put_allowed_.load()) {
          return std::nullopt;
        }
        queue_nempty_cv_.wait(guard);
      } else {
        break;
      }
    }

    auto val = std::move(buffer_.front());
    buffer_.pop_front();
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
      buffer_.clear();
    }
    queue_nempty_cv_.notify_all();
  }

  bool TryTake() {
    std::unique_lock guard(mutex_);
    return buffer_.empty();
  }

 private:
  std::deque<T> buffer_;
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable queue_nempty_cv_;
  twist::stdlike::atomic<bool> put_allowed_{true};
};

}  // namespace exe::tp
