#include <exe/executors/manual.hpp>

namespace exe::executors {

void ManualExecutor::Execute(TaskBase* task) {
  bag_of_tasks_.push(task);
}

// Run tasks

void ManualExecutor::RunTask() {
  auto task = std::move(bag_of_tasks_.front());
  bag_of_tasks_.pop();

  task->Run();
}

size_t ManualExecutor::RunAtMost(size_t limit) {
  size_t processed = 0;
  while (HasTasks() && processed != limit) {
    RunTask();
    ++processed;
  }
  return processed;
}

size_t ManualExecutor::Drain() {
  size_t processed = 0;
  while (HasTasks()) {
    RunTask();
    ++processed;
  }
  return processed;
}

}  // namespace exe::executors
