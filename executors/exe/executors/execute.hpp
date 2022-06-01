#pragma once

#include <exe/executors/executor.hpp>

namespace exe::executors {

/*
 * Usage:
 * Execute(thread_pool, []() {
 *   std::cout << "Hi" << std::endl;
 * });
 */

template <typename F>
struct Task : TaskBase {
  void Run() noexcept override {
    try {
      funct_();
    } catch (...) {
    }

    delete this;
  }

  void Discard() noexcept override {
    delete this;
  }

  explicit Task(F&& f) : funct_(std::forward<F>(f)) {
  }

 private:
  F funct_;
};

template <typename F>
void Execute(IExecutor& where, F&& f) {
  //  where.Execute(std::forward<F>(f));
  where.Execute(new Task<F>(std::forward<F>(f)));
}

}  // namespace exe::executors
