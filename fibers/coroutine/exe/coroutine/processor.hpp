#pragma once

#include <context/stack.hpp>
#include <exe/support/go_inclusions.hpp>
#include <exe/coroutine/standalone.hpp>
#include <wheels/support/defer.hpp>
#include <twist/util/thread_local.hpp>

#include <optional>

namespace exe::coroutine::processors {

template <typename T>
class Processor {
 public:
  explicit Processor(Routine routine)
      : stack_(AllocateStack()), coro_(std::move(routine)) {
    stored = new std::optional<T>;
    coro_.Resume();
  }

  void Send(T value) {
    Exchange(std::move(value));
  }

  void Close() {
    Exchange(std::nullopt);
  }

  // use inside processor routine
  static std::optional<T> Receive() {
    Coroutine::Suspend();
    return std::move(*stored);
  }

 private:
  void Exchange(std::optional<T> value) {
    auto prev = std::move(*stored);
    *stored = std::move(value);
    coro_.Resume();
    *stored = std::move(prev);
  }

  static context::Stack AllocateStack() {
    static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
    return context::Stack::AllocatePages(kStackPages);
  }

 private:
  context::Stack stack_;
  Coroutine coro_;
  static inline twist::util::ThreadLocalPtr<std::optional<T>> stored;
};

// Shortcut
template <typename T>
std::optional<T> Receive() {
  return Processor<T>::Receive();
}

}  // namespace exe::coroutine::processors
