#include <exe/fibers/core/api.hpp>
#include <exe/tp/thread_pool.hpp>

#include <wheels/test/test_framework.hpp>

#include <twist/test/test.hpp>

using exe::tp::ThreadPool;
using exe::fibers::Go;
using exe::fibers::self::Yield;

void ExpectPool(ThreadPool& pool) {
  ASSERT_EQ(ThreadPool::Current(), &pool);
}

TEST_SUITE(Fibers) {
  SIMPLE_TEST(JustWorks) {
    ThreadPool pool{3};

    bool done = false;

    Go(pool, [&]() {
      ExpectPool(pool);
      done = true;
    });

    pool.WaitIdle();

    ASSERT_TRUE(done);

    pool.Stop();
  }

  SIMPLE_TEST(Child) {
    ThreadPool pool{3};
    std::atomic<size_t> done{0};

    auto init = [&]() {
      ExpectPool(pool);

      Go([&]() {
        ExpectPool(pool);
        ++done;
      });

      ++done;
    };

    Go(pool, init);

    pool.WaitIdle();

    ASSERT_EQ(done.load(), 2);

    pool.Stop();
  }

  SIMPLE_TEST(RunInParallel) {
    ThreadPool pool{3};
    std::atomic<size_t> completed{0};

    auto sleeper = [&]() {
      std::this_thread::sleep_for(3s);
      completed.fetch_add(1);
    };

    wheels::StopWatch stop_watch;

    Go(pool, sleeper);
    Go(pool, sleeper);
    Go(pool, sleeper);

    pool.WaitIdle();

    ASSERT_EQ(completed.load(), 3);
    ASSERT_TRUE(stop_watch.Elapsed() < 3s + 500ms);

    pool.Stop();
  }

  SIMPLE_TEST(Yield) {
    std::atomic<int> value{0};

    auto check_value = [&]() {
      const int kLimit = 10;

      ASSERT_TRUE(value.load() < kLimit);
      ASSERT_TRUE(value.load() > -kLimit);
    };

    static const size_t kIterations = 12345;

    auto bull = [&]() {
      for (size_t i = 0; i < kIterations; ++i) {
        value.fetch_add(1);
        Yield();
        check_value();
      }
    };

    auto bear = [&]() {
      for (size_t i = 0; i < kIterations; ++i) {
        value.fetch_sub(1);
        Yield();
        check_value();
      }
    };

    // NB: 1 worker thread!
    ThreadPool pool{1};

    Go(pool, [&]() {
      Go(bull);
      Go(bear);
    });

    pool.WaitIdle();
    pool.Stop();
  }

  SIMPLE_TEST(Yield2) {
    ThreadPool pool{4};

    static const size_t kYields = 65536;

    auto tester = []() {
      for (size_t i = 0; i < kYields; ++i) {
        Yield();
      }
    };

    Go(pool, tester);
    Go(pool, tester);

    pool.WaitIdle();
    pool.Stop();
  }

  class ForkTester {
   public:
    ForkTester(size_t threads) : pool_(threads) {
    }

    size_t Explode(size_t d) {
      Go(pool_, MakeForker(d));

      pool_.WaitIdle();
      pool_.Stop();

      return leafs_.load();
    }

   private:
    exe::fibers::Routine MakeForker(size_t d) {
      return [this, d]() {
        if (d > 2) {
          Go(MakeForker(d - 2));
          Go(MakeForker(d - 1));
        } else {
          leafs_.fetch_add(1);
        }
      };
    }

   private:
    ThreadPool pool_;
    std::atomic<size_t> leafs_{0};
  };

//  // Under investigation
//  TEST(Forks, wheels::test::TestOptions().TimeLimit(10s).AdaptTLToSanitizer()) {
//    ForkTester tester{4};
//    // Respect ThreadSanitizer thread limit:
//    // Tid - 13 bits => 8192 threads
//    ASSERT_EQ(tester.Explode(20), 6765);
//  }

  SIMPLE_TEST(TwoPools1) {
    ThreadPool pool_1{4};
    ThreadPool pool_2{4};

    auto make_tester = [](ThreadPool& pool) {
      return [&pool]() {
        ExpectPool(pool);
      };
    };

    Go(pool_1, make_tester(pool_1));
    Go(pool_2, make_tester(pool_2));

    pool_1.WaitIdle();
    pool_2.WaitIdle();

    pool_1.Stop();
    pool_2.Stop();
  }

  SIMPLE_TEST(TwoPools2) {
    ThreadPool pool_1{4};
    ThreadPool pool_2{4};

    auto make_tester = [](ThreadPool& pool) {
      return [&pool]() {
        static const size_t kIterations = 1024;

        for (size_t i = 0; i < kIterations; ++i) {
          ExpectPool(pool);

          Yield();

          Go(pool, [&pool]() {
            ExpectPool(pool);
          });
        }
      };
    };

    Go(pool_1, make_tester(pool_1));
    Go(pool_2, make_tester(pool_2));

    pool_1.WaitIdle();
    pool_2.WaitIdle();

    pool_1.Stop();
    pool_2.Stop();
  }
}

RUN_ALL_TESTS()
