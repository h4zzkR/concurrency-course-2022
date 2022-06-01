#include <exe/tp/thread_pool.hpp>
#include <exe/fibers/core/api.hpp>

#include <iostream>

using namespace exe;

int main() {
  tp::ThreadPool scheduler{/*threads=*/4};

  for (size_t i = 0; i < 100500; ++i) {
    fibers::Go(scheduler, []() {
      for (size_t j = 0; j < 3; ++j) {
        fibers::self::Yield();
      }
    });
  }

  scheduler.WaitIdle();
  scheduler.Stop();

  return 0;
}
