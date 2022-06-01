#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

#include <asio/steady_timer.hpp>

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

void Fiber::Schedule() {
  // Not implemented
}

void Fiber::Yield() {
  // Not implemented
}

void Fiber::SleepFor(Millis /*delay*/) {
  // Not implemented
}

void Fiber::Step() {
  // Not implemented
}

Fiber& Fiber::Self() {
  std::abort();  // Not implemented
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& /*scheduler*/, Routine /*routine*/) {
  // Not implemented
}

void Go(Routine /*routine*/) {
  // Not implemented
}

namespace self {

void Yield() {
  // Not implemented
}

void SleepFor(Millis /*delay*/) {
  // Not implemented
}

}  // namespace self

}  // namespace exe::fibers
