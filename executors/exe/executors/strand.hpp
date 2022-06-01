//#pragma once
//
#include <exe/executors/executor.hpp>
#include <exe/executors/execute.hpp>
//#include <wheels/intrusive/forward_list.hpp>
//
//#include <exe/support/lockfree_stuff.hpp>
//
//#include <twist/stdlike/atomic.hpp>
//#include <twist/stdlike/mutex.hpp>
//
// namespace exe::executors {
//
//// Strand (serial executor, asynchronous mutex)
//// Executes (via underlying executor) tasks
//// non-concurrently and in FIFO order
//
// class Strand : public IExecutor {
//
// public:
//  explicit Strand(IExecutor& underlying) : executor_(underlying) {
//  }
//
//  // IExecutor
//  void Execute(TaskBase* task) override;
//
// private:
////  void EventLoop();
//  void ScheduleLoop();
//
//  IExecutor& executor_;
//  LFStack tasks_storage_;
//
//  twist::stdlike::atomic<bool> can_execute_{true};
//  twist::stdlike::mutex blocker;
//};
//
//}  // namespace exe::executors

#pragma once

#include <exe/executors/executor.hpp>
#include <queue>

#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/mutex.hpp>

namespace exe::executors {

// Strand (serial executor, asynchronous mutex)
// Executes (via underlying executor) tasks
// non-concurrently and in FIFO order

class Strand : public IExecutor {
  static const size_t kInitBatchSize = 1;
  static const size_t kMultiplier = 2;
  const double k_grab_rate_ = 1.5;

 public:
  explicit Strand(IExecutor& underlying) : executor_(underlying) {
  }

  // IExecutor
  void Execute(TaskBase* task) override;

 private:
  struct StrandBatch {
    void operator()();
    bool AddTask(TaskBase* task);

    explicit StrandBatch(size_t clip_size);
    explicit StrandBatch(size_t clip_size, TaskBase* task);
    StrandBatch(StrandBatch&& oth) = default;

    size_t clip_size;

    std::queue<TaskBase*> clip;
  };

  void EventLoop();

  template <typename Locker>
  void ScheduleLoop(std::unique_lock<Locker>& guarder);

  void PushBatch(TaskBase* task) {
    batches_.emplace(current_batch_size_, std::move(task));
    current_batch_size_ *= kMultiplier;
  }

  IExecutor& executor_;

  size_t current_batch_size_ = kInitBatchSize;
  size_t batch_grab_limit_;  // Do not occupy the thread

  bool can_reschedule_ = true;

  std::queue<StrandBatch> batches_;
  twist::stdlike::mutex batches_locker_;
};

}  // namespace exe::executors
