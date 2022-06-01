#pragma once

#include <queue>
#include <optional>
#include <wheels/support/defer.hpp>
#include <wheels/logging/logging.hpp>

namespace exe::support {

// TLDR: only for internal usage
// limit_ == 0 => unbounded
template <typename T>
class Queue2 {
 public:
  explicit Queue2(size_t limit = 0) {
    limit_ = limit;
  }

  Queue2(const Queue2<T>& q) = default;
  Queue2(Queue2<T>&& q) = default;

  std::optional<T> Pop() {
    if (Empty()) {
      return {};
    }

    auto tmp = std::move(queue_.front());
    queue_.pop();
    return tmp;
  }

  T* Glance() {
    if (Empty()) {
      return nullptr;
    }
    return &queue_.back();
  }

  bool Empty() const {
    return queue_.empty();
  }

  size_t Size() const {
    return queue_.size();
  }

  bool Push(T&& value) {
    if ((limit_ == 0u) || Size() < limit_) {
      queue_.push(std::forward<T>(value));
      return true;
    }
    //    LOG_SIMPLE("ERRPUSH");
    return false;
  }

  template <typename... Args>
  bool Emplace(Args&&... args) {
    if ((limit_ == 0u) || Size() < limit_) {
      queue_.template emplace(std::forward<Args>(args)...);
      return true;
    }
    return false;
  }

 private:
  size_t limit_;
  std::queue<T> queue_;
};

}  // namespace exe::support