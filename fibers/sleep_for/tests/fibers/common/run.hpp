#pragma once

#include <exe/fibers/core/api.hpp>

#include <asio.hpp>

#include <vector>
#include <thread>

template <typename F>
void RunScheduler(size_t threads, F init) {
  // I/O scheduler
  asio::io_context scheduler;

  // Spawn initial fiber

  bool done = false;

  exe::fibers::Go(scheduler, [init, &done]() {
    init();
    done = true;
  });

  // Run event loop

  std::vector<std::thread> runners;

  for (size_t i = 0; i < threads; ++i) {
    runners.emplace_back([&scheduler]() {
      scheduler.run();
    });
  }
  // Join runners
  for (auto& t : runners) {
    t.join();
  }

  ASSERT_TRUE(done);
}
