#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/fibers/core/awaiter.hpp>
#include <exe/fibers/core/handle.hpp>
#include <exe/coroutine/impl.hpp>
#include <context/stack.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler

struct IAwaiter;
class FiberHandle;

using Function = wheels::UniqueFunction<void()>;

class Fiber {
 public:
  void Schedule();
  Scheduler& GetSched();

  void Yield();

  /*
   * Stops fiber execution and calls Awaiter strategy
   * after stopping.
   */
  void Suspend(IAwaiter* strategy);
  void Suspend();
  void Resume();

  Fiber(Routine routine, Scheduler& sched, context::Stack stack);

  static FiberHandle Self();

 private:
  void Step();

 private:
  context::Stack stack_;
  coroutine::CoroutineImpl coro_;
  fibers::IAwaiter* step_callback_;
  Scheduler* sched_ = nullptr;
};

}  // namespace exe::fibers
