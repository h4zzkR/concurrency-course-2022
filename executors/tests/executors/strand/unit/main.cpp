#include <exe/executors/thread_pool.hpp>
#include <exe/executors/strand.hpp>
#include <exe/executors/manual.hpp>
#include <exe/executors/execute.hpp>

#include <wheels/test/test_framework.hpp>
#include <wheels/support/stop_watch.hpp>

#include <thread>
#include <deque>
#include <atomic>

using namespace exe::executors;
using namespace std::chrono_literals;

#include <wheels/logging/logging.hpp>

void ExpectThreadPool(ThreadPool& pool) {
  ASSERT_EQ(ThreadPool::Current(), &pool);
}

TEST_SUITE(Strand) {
  SIMPLE_TEST(JustWorks) {
    ThreadPool pool{4};
    Strand strand{pool};

    bool done = false;

    Execute(strand, [&]() {
      done = true;
    });

    pool.WaitIdle();

    ASSERT_TRUE(done);

    pool.Stop();
  }

  SIMPLE_TEST(Decorator) {
    ThreadPool pool{4};
    Strand strand(pool);

    bool done{false};

    for (size_t i = 0; i < 128; ++i) {
      Execute(strand, [&]() {
        ExpectThreadPool(pool);
        done = true;
      });
    }

    pool.WaitIdle();

    ASSERT_TRUE(done);

    pool.Stop();
  }

  SIMPLE_TEST(Counter) {
    ThreadPool pool{13};

    size_t counter = 0;

    Strand strand(pool);

    static const size_t kIncrements = 1234;

    for (size_t i = 0; i < kIncrements; ++i) {
      Execute(strand, [&]() {
        ExpectThreadPool(pool);
        ++counter;
        LOG_SIMPLE(counter);
      });
    };

    pool.WaitIdle();

    LOG_SIMPLE(counter << ' ' << kIncrements);
    fflush(stdout);
    ASSERT_EQ(counter, kIncrements);

    pool.Stop();
  }

  SIMPLE_TEST(Fifo) {
    ThreadPool pool{13};
    Strand strand(pool);

    size_t next_index = 0;

    static const size_t kTasks = 12345;

    for (size_t i = 0; i < kTasks; ++i) {
      Execute(strand, [&, i]() {
        ExpectThreadPool(pool);
//        LOG_SIMPLE(next_index << ' ' << i);
        ASSERT_EQ(next_index, i);
        ++next_index;
      });
    };

    pool.WaitIdle();

    ASSERT_EQ(next_index, kTasks);

    pool.Stop();
  }

  class Robot {
   public:
    explicit Robot(IExecutor& executor) : strand_(executor) {
    }

    void Kick() {
      Execute(strand_, [this]() {
        ++steps_;
      });
    }

    size_t Steps() const {
      return steps_;
    }

   private:
    Strand strand_;
    size_t steps_{0};
  };

  SIMPLE_TEST(ConcurrentStrands) {
    ThreadPool pool{16};

    static const size_t kStrands = 50;

    std::deque<Robot> robots;
    for (size_t i = 0; i < kStrands; ++i) {
      robots.emplace_back(pool);
    }

    static const size_t kKicks = 25;
    static const size_t kIterations = 25;

    for (size_t i = 0; i < kIterations; ++i) {
      for (size_t j = 0; j < kStrands; ++j) {
        for (size_t k = 0; k < kKicks; ++k) {
          robots[j].Kick();
        }
      }
    }

    pool.WaitIdle();

    for (size_t i = 0; i < kStrands; ++i) {
      ASSERT_EQ(robots[i].Steps(), kKicks * kIterations);
    }

    pool.Stop();
  }

  SIMPLE_TEST(ConcurrentExecutes) {
    ThreadPool workers{2};
    Strand strand{workers};

    ThreadPool clients{4};

    static const size_t kTasks = 1024;

    size_t task_count{0};

    for (size_t i = 0; i < kTasks; ++i) {
      Execute(clients, [&]() {
        Execute(strand, [&]() {
          ExpectThreadPool(workers);
          std::cout << "Task executed!" << std::endl;
          ++task_count;
        });
      });
    }

    clients.WaitIdle();
    workers.WaitIdle();

    ASSERT_EQ(task_count, kTasks);

    workers.Stop();
    clients.Stop();
  }

  SIMPLE_TEST(StrandOverManual) {
    ManualExecutor manual;
    Strand strand{manual};

    static const size_t kTasks = 117;

    size_t tasks = 0;

    for (size_t i = 0; i < kTasks; ++i) {
      Execute(strand, [&]() {
        ++tasks;
      });
    }

    manual.Drain();

    ASSERT_EQ(tasks, kTasks);
  }

  SIMPLE_TEST(Batching) {
    ManualExecutor manual;
    Strand strand{manual};

    static const size_t kTasks = 100;

    size_t completed = 0;
    for (size_t i = 0; i < kTasks; ++i) {
      Execute(strand, [&completed]() {
        ++completed;
      });
    };

    size_t tasks = manual.Drain();
    ASSERT_LT(tasks, 5);
  }

  SIMPLE_TEST(StrandOverStrand) {
    ThreadPool pool{4};

    Strand strand_1(pool);
    Strand strand_2((IExecutor&)strand_1);

    static const size_t kTasks = 17;

    size_t tasks = 0;

    for (size_t i = 0; i < kTasks; ++i) {
      Execute(strand_2, [&tasks]() {
        ++tasks;
      });
    }

    pool.WaitIdle();

    ASSERT_EQ(tasks, kTasks);

    pool.Stop();
  }

  SIMPLE_TEST(DoNotOccupyThread) {
    ThreadPool pool{1};
    Strand strand{pool};

    Execute(pool, []() {
      std::this_thread::sleep_for(1s);
    });

    std::atomic<bool> stop_requested{false};

    auto snooze = []() {
      std::this_thread::sleep_for(10ms);
    };

    for (size_t i = 0; i < 100; ++i) {
      Execute(strand, snooze);
    }

    Execute(pool, [&stop_requested]() {
      stop_requested.store(true);
    });

    while (!stop_requested.load()) {
      Execute(strand, snooze);
      std::this_thread::sleep_for(10ms);
    }

    pool.WaitIdle();
    pool.Stop();
  }

  SIMPLE_TEST(NonBlockingExecute) {
    ThreadPool pool{1};
    Strand strand{pool};

    Execute(strand, [&]() {
      // Bubble
      std::this_thread::sleep_for(3s);
    });

    std::this_thread::sleep_for(256ms);

    {
      wheels::StopWatch stop_watch;
      Execute(strand, [&]() {
        // Do nothing
      });

      ASSERT_LE(stop_watch.Elapsed(), 100ms);
    }

    pool.WaitIdle();
    pool.Stop();
  }
}

RUN_ALL_TESTS()
