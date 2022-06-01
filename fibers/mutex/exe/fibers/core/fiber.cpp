#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>
#include <wheels/logging/logging.hpp>

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<Fiber> fiber;

//////////////////////////////////////////////////////////////////////

void Fiber::Schedule() {
  sched_->Submit([this]() {
    Step();
  });
}

void Fiber::Yield() {
  auto aw = ReScheduleAwaiter(FiberHandle(this));
  Suspend(&aw);
}

void Fiber::Step() {
  fiber = this;
  coro_.Resume();
  if (coro_.IsCompleted()) {
    //    LOG_SIMPLE("ROUTINE END");
    delete this;
  } else {
    (*step_callback_)();
  }
}

FiberHandle Fiber::Self() {
  return FiberHandle(fiber);
}

Scheduler& Fiber::GetSched() {
  return *sched_;
}

Fiber::Fiber(Routine routine, Scheduler& shed, context::Stack stack)
    : stack_(std::move(stack)), coro_(std::move(routine), stack_.View()) {
  sched_ = &shed;
}

/*
 * Strategy must be alive in Step
 */
void Fiber::Suspend(IAwaiter* strategy) {
  step_callback_ = strategy;
  coro_.Suspend();
}

void Fiber::Suspend() {
  auto aw = DoNothingAwaiter{};
  Suspend(&aw);
}

void Fiber::Resume() {
  coro_.Resume();
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  Fiber* fib = new Fiber(std::move(routine), scheduler, AllocateStack());
  fib->Schedule();
}

void Go(Routine routine) {
  Go(fiber->GetSched(), std::move(routine));
}

namespace self {

void Yield() {
  fiber->Yield();
}

void Suspend(IAwaiter* strategy) {
  fiber->Suspend(strategy);
}

void Suspend() {
  fiber->Suspend();
}

}  // namespace self

}  // namespace exe::fibers
