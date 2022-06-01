#pragma once

#include <exe/tp/blocking_queue.hpp>
#include <exe/tp/task.hpp>
#include <twist/stdlike/thread.hpp>
#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/condition_variable.hpp>
#include <exe/support/go_inclusions.hpp>

#include <vector>

namespace exe::tp {

using ::tp::Task;

// Fixed-size pool of worker threads

class ThreadPool {
 public:
  explicit ThreadPool(size_t workers, size_t id = 0);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Schedules task for execution in one of the worker threads
  void Submit(Task task);

  // Waits until outstanding work count has reached zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  // Pending tasks will be discarded
  void Stop();

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

 public:
  size_t id_ = 0;

 private:
  void StartWorkerThreads(size_t count);
  void WorkerRoutine();

  // Worker threads, task queue, etc
  UnboundedBlockingQueue<tp::Task> tasks_;
  std::vector<twist::stdlike::thread> workers_;
  twist::stdlike::detail::WaitGroup wg_;

  twist::stdlike::atomic<bool> stopped_{false};
};

inline ThreadPool* Current() {
  return ThreadPool::Current();
}

inline void Submit(Task task) {
  tp::Current()->Submit(std::move(task));
}

}  // namespace exe::tp
