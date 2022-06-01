#include <wheels/test/test_framework.hpp>

#include <futures/promise.hpp>

#include <string>
#include <thread>
#include <variant>

using stdlike::Promise;
using stdlike::Future;

using namespace std::chrono_literals;

TEST_SUITE(Futures) {
  struct MoveOnly {
    MoveOnly() = default;

    // Non-copyable
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;

    // Movable
    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator=(MoveOnly&&) = default;
  };

  SIMPLE_TEST(MoveOnly) {
    Promise<MoveOnly> p;
    auto f = p.MakeFuture();

    std::thread producer([p = std::move(p)]() mutable {
      p.SetValue(MoveOnly{});
    });

    f.Get();
    producer.join();
  }

  SIMPLE_TEST(NonDefaultConstructable) {
    struct NonDefaultConstructable {
      explicit NonDefaultConstructable(int) {
      }
    };
    Promise<NonDefaultConstructable> p;
  }

  SIMPLE_TEST(MonoState) {
    Promise<std::monostate> p;
    auto f = p.MakeFuture();
    bool ok = false;

    std::thread producer([p = std::move(p), &ok]() mutable {
      std::this_thread::sleep_for(1s);
      ok = true;
      p.SetValue({});
    });

    f.Get();
    ASSERT_TRUE(ok);

    producer.join();
  }

  class TestException : public std::runtime_error {
   public:
    TestException() : std::runtime_error("Test") {
    }
  };

  SIMPLE_TEST(ExceptionPtr) {
    Promise<std::exception_ptr> p;
    auto f = p.MakeFuture();

    std::thread producer([p = std::move(p)]() mutable {
      std::this_thread::sleep_for(1s);
      try {
        throw TestException();
      } catch (...) {
        p.SetValue(std::current_exception());
      }
    });

    auto ex = f.Get();
    ASSERT_THROW(std::rethrow_exception(ex), TestException);

    producer.join();
  }

  SIMPLE_TEST(ExceptionPtrThrow) {
    Promise<std::exception_ptr> p;
    auto f = p.MakeFuture();

    std::thread producer([p = std::move(p)]() mutable {
      std::this_thread::sleep_for(1s);
      try {
        throw TestException();
      } catch (...) {
        p.SetException(std::current_exception());
      }
    });

    std::exception_ptr ex;
    ASSERT_THROW(ex = f.Get(), TestException);
    producer.join();
  }
}

RUN_ALL_TESTS()
