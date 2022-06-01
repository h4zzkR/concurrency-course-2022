#include <wheels/test/test_framework.hpp>

#include <exe/tp/thread_pool.hpp>
#include <exe/fibers/sync/mutex.hpp>

#include <wheels/support/cpu_time.hpp>

#include <atomic>
#include <chrono>
#include <thread>

using namespace exe::tp;
using namespace exe::fibers;

using namespace std::chrono_literals;

TEST_SUITE(Mutex) {
  SIMPLE_TEST(Counter) {
    ThreadPool scheduler{4};

    Mutex mutex;
    size_t cs = 0;

    for (size_t i = 0; i < 10; ++i) {
      Go(scheduler, [&]() {
        for (size_t j = 0; j < 1024; ++j) {
          std::lock_guard guard(mutex);
          ++cs;
        }
      });
    }

    scheduler.WaitIdle();

    ASSERT_EQ(cs, 1024 * 10);

    scheduler.Stop();
  }

  SIMPLE_TEST(Blocking) {
    ThreadPool scheduler{4};

    Mutex mutex;

    wheels::ProcessCPUTimer timer;

    Go(scheduler, [&]() {
      mutex.Lock();
      std::this_thread::sleep_for(1s);
      mutex.Unlock();
    });

    Go(scheduler, [&]() {
      mutex.Lock();
      mutex.Unlock();
    });

    scheduler.WaitIdle();

    ASSERT_TRUE(timer.Elapsed() < 100ms);

    scheduler.Stop();
  }
}

RUN_ALL_TESTS()