#include <exe/coroutine/generator.hpp>

#include <wheels/test/test_framework.hpp>

#include <memory>
#include <string>
#include <vector>
#include <sstream>

using namespace exe::coroutine::generators;

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Generator) {
  SIMPLE_TEST(JustWorks) {
    Generator<std::string> g([&]() {
      Send<std::string>("Hello");
      Send<std::string>("World");
    });

    {
      auto next = g.Receive();
      ASSERT_TRUE(next);
      ASSERT_EQ(*next, "Hello");
    }

    {
      auto next = g.Receive();
      ASSERT_TRUE(next);
      ASSERT_EQ(*next, "World");
    }

    {
      auto next = g.Receive();
      ASSERT_FALSE(next);
    }
  }

  SIMPLE_TEST(Countdown) {
    Generator<int> countdown([]() {
      for (int i = 10; i >= 0; --i) {
        Send<int>(i);
      }
    });

    for (int i = 10; i >= 0; --i) {
      auto next = countdown.Receive();
      ASSERT_TRUE(next);
      ASSERT_EQ(*next, i);
    }

    ASSERT_FALSE(countdown.Receive());
  }

  void Reverse(Generator<std::string>& g) {
    if (auto next = g.Receive()) {
      Reverse(g);
      Send(*next);
    }
  }

  SIMPLE_TEST(Reverse) {
    Generator<std::string> generator([]() {
      Send<std::string>("Hello");
      Send<std::string>(",");
      Send<std::string>("World");
      Send<std::string>("!");
    });

    Generator<std::string> reverser([&generator]() {
      Reverse(generator);
    });

    std::stringstream out;
    while (auto msg = reverser.Receive()) {
      out << *msg;
    }

    ASSERT_EQ(out.str(), "!World,Hello");
  }
}

RUN_ALL_TESTS()