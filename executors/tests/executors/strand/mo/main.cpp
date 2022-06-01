#include <exe/executors/thread_pool.hpp>
#include <exe/executors/strand.hpp>
#include <exe/executors/execute.hpp>

#include <twist/test/test.hpp>

#include <wheels/test/test_framework.hpp>
#include <wheels/test/util.hpp>

#include <thread>

using namespace exe::executors;
using namespace std::chrono_literals;

/////////////////////////////////////////////////////////////////////

class OnePassBarrier {
 public:
  explicit OnePassBarrier(size_t threads) : total_(threads) {
  }

  void PassThrough() {
    arrived_.fetch_add(1);
    while (arrived_.load() < total_) {
      std::this_thread::yield();
    }
  }

 private:
  size_t total_{0};
  std::atomic<size_t> arrived_{0};
};

void ScheduleExecuteRace() {
  ThreadPool pool{1};

  while (wheels::test::KeepRunning()) {
    Strand strand(pool);
    OnePassBarrier barrier{2};

    size_t done = 0;

    Execute(strand, [&done, &barrier] {
      ++done;
      barrier.PassThrough();
    });

    barrier.PassThrough();

    Execute(strand, [&done] {
      ++done;
    });

    pool.WaitIdle();

    ASSERT_EQ(done, 2);
  }

  pool.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(StrandMemoryOrderings) {
  TWIST_TEST_TL(ScheduleExecuteRace, 5s) {
    ScheduleExecuteRace();
  }
}

RUN_ALL_TESTS()
