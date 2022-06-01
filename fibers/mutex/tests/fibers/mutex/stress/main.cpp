#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <twist/test/test.hpp>
#include <twist/test/util/plate.hpp>

#include <exe/tp/thread_pool.hpp>
#include <exe/fibers/sync/mutex.hpp>

#include <atomic>
#include <chrono>

using namespace exe;
using namespace std::chrono_literals;

//////////////////////////////////////////////////////////////////////

void StressTest1(size_t fibers) {
  tp::ThreadPool scheduler{4};

  fibers::Mutex mutex;
  twist::test::util::Plate plate;

  for (size_t i = 0; i < fibers; ++i) {
    fibers::Go(scheduler, [&]() {
      while (wheels::test::KeepRunning()) {
        mutex.Lock();
        plate.Access();
        mutex.Unlock();
      }
    });
  }

  scheduler.WaitIdle();

  std::cout << "# critical sections: " << plate.AccessCount() << std::endl;
  ASSERT_TRUE(plate.AccessCount() > 12345);

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

void StressTest2(size_t fibers) {
  tp::ThreadPool scheduler{4};

  while (wheels::test::KeepRunning()) {
    fibers::Mutex mutex;
    std::atomic<size_t> cs{0};

    for (size_t j = 0; j < fibers; ++j) {
      fibers::Go(scheduler, [&]() {
        mutex.Lock();
        ++cs;
        mutex.Unlock();
      });
    }

    scheduler.WaitIdle();

    ASSERT_EQ(cs, fibers);
  }

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Mutex) {
  TWIST_TEST_TL(Stress_1_1, 5s) {
    StressTest1(/*fibers=*/4);
  }

  TWIST_TEST_TL(Stress_1_2, 5s) {
    StressTest1(/*fibers=*/16);
  }

  TWIST_TEST_TL(Stress_1_3, 5s) {
    StressTest1(/*fibers=*/100);
  }

  TWIST_TEST_TL(Stress_2_1, 5s) {
    StressTest2(/*fibers=*/2);
  }

  TWIST_TEST_TL(Stress_2_2, 5s) {
    StressTest2(/*fibers=*/3);
  }
}

RUN_ALL_TESTS()
