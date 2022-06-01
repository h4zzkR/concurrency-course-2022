#pragma once

#include "twist/stdlike/thread.hpp"
#include "twist/stdlike/atomic.hpp"
#include "twist/stdlike/condition_variable.hpp"
#include <wheels/support/function.hpp>

namespace twist::stdlike::detail {

using Routine = wheels::UniqueFunction<void()>;

class WaitGroup {
 public:
  WaitGroup() = default;
  void Add();
  void Done();
  void Wait();

 private:
  twist::stdlike::atomic<uint32_t> routines_{0};
};

void WaitGroup::Add() {
  routines_.fetch_add(1);
}

void WaitGroup::Done() {
  routines_.fetch_sub(1);
  routines_.notify_one();
}

void WaitGroup::Wait() {
  uint32_t current_routines;
  while ((current_routines = routines_.load()) != 0) {
    routines_.wait(current_routines);
  }
}
}  // namespace twist::stdlike::detail
