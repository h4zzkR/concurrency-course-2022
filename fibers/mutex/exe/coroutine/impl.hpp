#pragma once

#include <exe/coroutine/routine.hpp>

#include <context/context.hpp>

#include <wheels/memory/view.hpp>

#include <exception>

namespace exe::coroutine {

// Stackful asymmetric coroutine impl
// - Does not manage stacks
// - Unsafe Suspend
// Base for standalone coroutines, processors, fibers

class CoroutineImpl : public ::context::ITrampoline {
 public:
  CoroutineImpl(Routine routine, wheels::MutableMemView stack);

  // Context: Caller
  void Resume();

  // Context: Coroutine
  void Suspend();

  // Context: Caller
  bool IsCompleted() const;

 private:
  // context::ITrampoline
  [[noreturn]] void Run() override;

 private:
  std::exception_ptr eptr_;
  bool is_completed_ = false;

  Routine routine_;
  context::ExecutionContext context_, context_buf_;
  wheels::MutableMemView stack_;
};

}  // namespace exe::coroutine
