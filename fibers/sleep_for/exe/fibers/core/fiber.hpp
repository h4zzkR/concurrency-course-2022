#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/coroutine/impl.hpp>

#include <context/stack.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler

class Fiber {
 public:
  // ~ System calls

  void Schedule();

  void Yield();
  void SleepFor(Millis delay);

  void Suspend();
  void Resume();

  static Fiber& Self();

 private:
  // Task
  void Step();

  void Dispatch();

 private:
  // ???
};

}  // namespace exe::fibers
