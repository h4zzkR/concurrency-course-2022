#pragma once

#include <exe/executors/executor.hpp>
#include <queue>

namespace exe::executors {

// Single-threaded task queue

class ManualExecutor : public IExecutor {
 public:
  // IExecutor
  void Execute(TaskBase* task) override;

  // Run tasks

  // Run at most `limit` tasks from queue
  // Returns number of completed tasks
  size_t RunAtMost(size_t limit);

  // Run next task if queue is not empty
  bool RunNext() {
    return RunAtMost(1) == 1;
  }

  // Run tasks until queue is empty
  // Returns number of completed tasks
  // Post-condition: HasTasks() == false
  size_t Drain();

  size_t TaskCount() const {
    return bag_of_tasks_.size();
  }

  bool HasTasks() const {
    return !bag_of_tasks_.empty();
  }

 private:
  // Before using, make sure queue is not empty
  void RunTask();

  std::queue<TaskBase*> bag_of_tasks_;
};

}  // namespace exe::executors
