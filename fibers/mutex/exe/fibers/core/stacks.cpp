#include <exe/fibers/core/stacks.hpp>
#include <vector>
#include <stack>

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/atomic.hpp>

using context::Stack;

namespace exe::fibers {

// TODO: allocation problem, must rewrite on stack

//////////////////////////////////////////////////////////////////////

class StackAllocator {
 public:
  Stack Allocate() {
    //    std::lock_guard lock(mutex_);
    //
    //    if (pool_.size() < free_ptr_ * 2) {
    //      for (size_t i = 0; i < pool_.size(); ++i) {
    //        pool_.emplace_back(AllocateNewStack());
    //      }
    //    }
    //
    //    return std::move(pool_[free_ptr_++]);

    return AllocateNewStack();
  }

  explicit StackAllocator(const size_t buckets) {
    pool_.resize(buckets);
    for (size_t i = 0; i < buckets; ++i) {
      pool_[i] = AllocateNewStack();
    }
  }

  StackAllocator() : StackAllocator(kInitialBucketsCnt) {
  }

  void Release(Stack stack) {
    std::lock_guard lock(mutex_);
    if (free_ptr_ > 0) {
      pool_[--free_ptr_] = std::move(stack);
    } else {
      pool_.emplace_back(std::move(stack));
    }
  }

 private:
  static Stack AllocateNewStack() {
    static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
    return Stack::AllocatePages(kStackPages);
  }

 private:
  // Pool
  static constexpr size_t kInitialBucketsCnt = 64;
  std::vector<Stack> pool_;
  twist::stdlike::mutex mutex_;

  size_t free_ptr_ = 0;
};

//////////////////////////////////////////////////////////////////////

StackAllocator allocator;

context::Stack AllocateStack() {
  return allocator.Allocate();
}

void ReleaseStack(context::Stack stack) {
  allocator.Release(std::move(stack));
}

}  // namespace exe::fibers
