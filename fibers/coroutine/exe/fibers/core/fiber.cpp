#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <wheels/support/defer.hpp>
#include <twist/util/thread_local.hpp>

#include <wheels/logging/logging.hpp>
using wheels::LogMessageSimple;

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<Fiber> fiber;

//////////////////////////////////////////////////////////////////////

void Fiber::Schedule() {
  if (coro_.IsCompleted()) {
    Discard();
    return;
  }

  sched->Submit(this);
}

Fiber::Run() {
  Step();
}

Fiber::Discard() {
  // No-op
}

void Fiber::Yield() {
  coro_.Suspend();
}

void Fiber::Step() {
  fiber = this;
  coro_.Resume();
  Schedule();
}

Fiber& Fiber::Self() {
  return *fiber;
}

Scheduler& Fiber::GetSched() {
  return *sched;
}

Fiber::Fiber(Routine routine, Scheduler& shed, context::Stack stack)
    : stack_(std::move(stack)), coro_(std::move(routine), stack_.View()) {
  sched = &shed;
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  Fiber* fib = new Fiber(std::move(routine), scheduler, AllocateStack());
  fib->Schedule();
}

void Go(Routine routine) {
  Go(Fiber::Self().GetSched(), std::move(routine));
}

namespace self {

void Yield() {
  fiber->Yield();
}

}  // namespace self

}  // namespace exe::fibers
