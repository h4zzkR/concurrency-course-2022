#include <exe/executors/tp/compute/thread_pool.hpp>
#include <twist/util/thread_local.hpp>

#include <wheels/logging/logging.hpp>
using wheels::LogMessageSimple;

namespace exe::executors::tp::compute {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<ThreadPool> pool;

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t workers) {
  StartWorkerThreads(workers);
}

ThreadPool::~ThreadPool() {
  assert(stopped_.load());
}

void ThreadPool::Execute(TaskBase* task) {
  wg_.Add();
  tasks_.Put(task);
  //  tasks_.Put([this, task = std::move(task)]() mutable {
  //    try {
  //      task();
  //    } catch (...) {
  //    }
  //    wg_.Done();
  //  });
}

void ThreadPool::WaitIdle() {
  wg_.Wait();
}

void ThreadPool::Stop() {
  stopped_.store(true);
  tasks_.Cancel();
  for (auto& worker : workers_) {
    worker.join();
  }
}

ThreadPool* ThreadPool::Current() {
  return pool;
}

void ThreadPool::StartWorkerThreads(size_t count) {
  for (size_t i = 0; i < count; ++i) {
    workers_.emplace_back([this]() {
      WorkerRoutine();
    });
  }
}

void ThreadPool::WorkerRoutine() {
  while (auto task = tasks_.Take()) {
    // wait if necessary

    if (stopped_.load()) {
      return;
    }

    pool = this;
    task->Run();  // self destruction is here
    wg_.Done();
  }
}

}  // namespace exe::executors::tp::compute
