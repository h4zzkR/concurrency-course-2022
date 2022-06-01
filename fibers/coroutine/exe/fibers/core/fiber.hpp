#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/coroutine/impl.hpp>
#include <context/stack.hpp>

#include <twist/util/thread_local.hpp>
#include <exe/tp/thread_pool.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler (Thread pool)

class Fiber {
 public:
  // ~ System calls
  void Schedule();
  void Yield();
  Scheduler& GetSched();

  Fiber(Routine routine, Scheduler& sched, context::Stack stack);
  static Fiber& Self();

 private:
  // Task
  void Step();

 private:
  context::Stack stack_;
  coroutine::CoroutineImpl coro_;

 public:
  Scheduler* sched = nullptr;
};

}  // namespace exe::fibers
