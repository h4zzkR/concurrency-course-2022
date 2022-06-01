#include <exe/fibers/core/handle.hpp>
#include <exe/fibers/core/fiber.hpp>
#include <wheels/support/assert.hpp>
#include <utility>

namespace exe::fibers {

Fiber* FiberHandle::Release() {
  return std::exchange(fiber_, nullptr);
}
void FiberHandle::Schedule() {
  Fiber* fib = Release();
  fib->Schedule();
}
void FiberHandle::Resume() {
  fiber_->Resume();
}
FiberHandle FiberHandle::Invalid() {
  return FiberHandle();
}
bool FiberHandle::IsValid() const {
  return fiber_ != nullptr;
}

}  // namespace exe::fibers
