#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <twist/test/test.hpp>
#include <twist/test/util/plate.hpp>

#include <exe/tp/thread_pool.hpp>
#include <exe/fibers/sync/mutex.hpp>
#include <exe/fibers/sync/condvar.hpp>

#include <atomic>
#include <chrono>

using namespace exe;
using namespace std::chrono_literals;

//////////////////////////////////////////////////////////////////////

class Event {
 public:
  void Wait() {
    std::unique_lock lock(mutex_);
    while (!ready_) {
      waiters_.Wait(lock);
    }
  }

  void Fire() {
    std::lock_guard guard(mutex_);
    ready_ = true;
    waiters_.NotifyAll();
  }

 private:
  fibers::Mutex mutex_;
  fibers::CondVar waiters_;
  bool ready_{false};
};

//////////////////////////////////////////////////////////////////////

void EventStressTest() {
  tp::ThreadPool scheduler{/*threads=*/4};

  while (wheels::test::KeepRunning()) {
    Event event;
    std::atomic<size_t> oks{0};

    exe::fibers::Go(scheduler, [&]() {
      event.Wait();
      ++oks;
    });

    fibers::Go(scheduler, [&]() {
      event.Fire();
    });

    fibers::Go(scheduler, [&]() {
      event.Wait();
      ++oks;
    });

    scheduler.WaitIdle();

    ASSERT_EQ(oks.load(), 2);
  }

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

class Semaphore {
 public:
  explicit Semaphore(size_t init)
  : permits_(init) {
  }

  void Acquire() {
    std::unique_lock lock(mutex_);
    while (permits_ == 0) {
      has_permits_.Wait(lock);
    }
    --permits_;
  }

  void Release() {
    std::lock_guard guard(mutex_);
    ++permits_;
    has_permits_.NotifyOne();
  }

 private:
  fibers::Mutex mutex_;
  fibers::CondVar has_permits_;
  size_t permits_;
};

//////////////////////////////////////////////////////////////////////

void SemaphoreStressTest(size_t permits, size_t fibers) {
  tp::ThreadPool scheduler{/*threads=*/4};

  while (wheels::test::KeepRunning()) {
    Semaphore sema{/*init=*/permits};

    std::atomic<size_t> access_count{0};
    std::atomic<size_t> load{0};

    for (size_t i = 0; i < fibers; ++i) {
      fibers::Go(scheduler, [&]() {
        sema.Acquire();

        ++access_count;

        {
          size_t curr_load = load.fetch_add(1) + 1;
          ASSERT_TRUE(curr_load <= permits);

          twist::test::InjectFault();

          load.fetch_sub(1);
        }

        sema.Release();
      });
    };

    scheduler.WaitIdle();

    ASSERT_EQ(access_count.load(), fibers);
  }

  scheduler.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(CondVar) {
  TWIST_TEST_TL(EventStress, 5s) {
    EventStressTest();
  }

  TWIST_TEST_TL(SemaphoreStress_1, 5s) {
    SemaphoreStressTest(/*permits=*/1, /*fibers=*/2);
  }

  TWIST_TEST_TL(SemaphoreStress_2, 5s) {
    SemaphoreStressTest(/*permits=*/2, /*fibers=*/4);
  }

  TWIST_TEST_TL(SemaphoreStress_3, 5s) {
    SemaphoreStressTest(/*permits=*/2, /*fibers=*/10);
  }

  TWIST_TEST_TL(SemaphoreStress_4, 5s) {
    SemaphoreStressTest(/*permits=*/5, /*fibers=*/16);
  }
}

RUN_ALL_TESTS()
