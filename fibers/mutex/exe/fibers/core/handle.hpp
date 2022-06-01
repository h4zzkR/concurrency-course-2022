#pragma once

namespace exe::fibers {

// Lightweight non-owning handle to a _suspended_ fiber object

class Fiber;

class FiberHandle {
  friend Fiber;

 public:
  FiberHandle() : FiberHandle(nullptr) {
  }

  static FiberHandle Invalid();
  bool IsValid() const;

  // Schedule to an associated scheduler
  void Schedule();
  // Resume immediately in the current thread
  void Resume();

 private:
  explicit FiberHandle(Fiber* p_fiber) : fiber_(p_fiber) {
  }

  Fiber* Release();

 private:
  Fiber* fiber_;
};

}  // namespace exe::fibers
