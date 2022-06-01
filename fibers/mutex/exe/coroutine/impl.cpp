#include <exe/coroutine/impl.hpp>

#include <wheels/support/assert.hpp>
#include <wheels/support/compiler.hpp>

namespace exe::coroutine {

CoroutineImpl::CoroutineImpl(Routine routine, wheels::MutableMemView stack)
    : routine_(std::move(routine)), stack_(std::move(stack)) {
  context_.Setup(stack_, this);
}

void CoroutineImpl::Run() {
  try {
    routine_();
  } catch (...) {
    eptr_ = std::current_exception();
  }

  is_completed_ = true;
  context_.ExitTo(context_buf_);

  WHEELS_UNREACHABLE();
}

void CoroutineImpl::Resume() {
  context_buf_.SwitchTo(context_);
  if (eptr_) {
    std::rethrow_exception(eptr_);
  }
}

void CoroutineImpl::Suspend() {
  context_.SwitchTo(context_buf_);
}

bool CoroutineImpl::IsCompleted() const {
  return is_completed_;
}

}  // namespace exe::coroutine
