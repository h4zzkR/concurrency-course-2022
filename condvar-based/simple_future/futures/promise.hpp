#pragma once

#include <futures/future.hpp>

#include <memory>

namespace stdlike {

template <typename T>
class Promise {
 public:
  Promise() {
  }

  // Non-copyable
  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;

  // Movable
  Promise(Promise&&) = default;
  Promise& operator=(Promise&&) = default;

  // One-shot
  Future<T> MakeFuture() {
    throw std::runtime_error("Not implemented");
  }

  // One-shot
  // Fulfill promise with value
  void SetValue(T /*value*/) {
    // Not implemented
  }

  // One-shot
  // Fulfill promise with exception
  void SetException(std::exception_ptr /*ex*/) {
    // Not implemented
  }

 private:
  // ???
};

}  // namespace stdlike
