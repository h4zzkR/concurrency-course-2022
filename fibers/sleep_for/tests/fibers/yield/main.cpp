#include <wheels/test/test_framework.hpp>

#include <exe/fibers/core/api.hpp>

#include "../common/run.hpp"

using namespace exe;
using namespace std::chrono_literals;

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Yield) {
  SIMPLE_TEST(JustWorks) {
    RunScheduler(/*threads=*/1, []() {
      fibers::self::Yield();
    });
  }

  SIMPLE_TEST(TwoFibers) {
    RunScheduler(/*threads=*/1, []() {
      size_t step = 0;
      fibers::Go([&]() {
        for (size_t j = 1; j < 7; ++j) {
          step = j;
          fibers::self::Yield();
        }
      });

      for (size_t i = 1; i < 7; ++i) {
        fibers::self::Yield();
        ASSERT_EQ(step, i);
      }
    });
  }

  TEST(Stress, wheels::test::TestOptions().TimeLimit(10s).AdaptTLToSanitizer()) {
    RunScheduler(/*threads=*/4, []() {
      for (size_t i = 0; i < 127; ++i) {
        fibers::Go([]() {
          for (size_t j = 0; j < 12345; ++j) {
            fibers::self::Yield();
          }
        });
      }
    });
  }
}

RUN_ALL_TESTS()
