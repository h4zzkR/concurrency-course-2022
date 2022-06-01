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

TEST_SUITE(MutexExtra) {
  SIMPLE_TEST(UnlockFork) {
    ThreadPool scheduler{4};

    Mutex mutex;

    Go(scheduler, [&]() {
      mutex.Lock();
      std::this_thread::sleep_for(1s);
      mutex.Unlock();
    });

    std::this_thread::sleep_for(128ms);

    for (size_t i = 0; i < 3; ++i) {
      Go(scheduler, [&]() {
        mutex.Lock();
        mutex.Unlock();
        std::this_thread::sleep_for(5s);
      });
    }

    scheduler.WaitIdle();
    scheduler.Stop();
  }
}

RUN_ALL_TESTS()
