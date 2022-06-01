#pragma once

#include <exe/coroutine/impl.hpp>
#include <context/stack.hpp>

#include <exe/coroutine/processor.hpp>
#include <optional>

namespace exe::coroutine::generators {

template <typename T>
class Generator {
 public:
  explicit Generator(Routine routine)
      : stack_(AllocateStack()), co_(std::move(routine)) {
    stored = new std::optional<T>;
  }

  // Pull
  std::optional<T> Receive() {
    co_.Resume();
    return std::move(*stored);
  }

  static void Send(T value) {
    Exchange(std::move(value));
  }

 private:
  // Intentionally naive and inefficient
  static context::Stack AllocateStack() {
    static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
    return context::Stack::AllocatePages(kStackPages);
  }

  static void Exchange(std::optional<T> value) {
    auto prev = std::move(*stored);
    *stored = std::move(value);
    Coroutine::Suspend();
    *stored = std::move(prev);
  }

 private:
  context::Stack stack_;
  Coroutine co_;
  static inline twist::util::ThreadLocalPtr<std::optional<T>> stored;
};

// Shortcut
template <typename T>
void Send(T value) {
  Generator<T>::Send(std::move(value));
}

}  // namespace exe::coroutine::generators
