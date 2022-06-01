#include <exe/coroutine/processor.hpp>

#include <wheels/test/test_framework.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace exe::coroutine::processors;

TEST_SUITE(Processor) {
  SIMPLE_TEST(JustWorks) {
    bool done = false;

    Processor<std::string> p([&]() {
      auto message = Receive<std::string>();
      ASSERT_EQ(*message, "Hi");

      ASSERT_FALSE(Receive<std::string>());

      done = true;
    });

    p.Send("Hi");
    p.Close();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(PipeLine) {
    std::string output;

    Processor<std::string> p3([&]() {
      while (auto message = Receive<std::string>()) {
        output += *message;
      }
    });

    Processor<std::string> p2([&p3]() {
      while (auto message = Receive<std::string>()) {
        p3.Send(std::move(*message));
      }
      p3.Close();
    });

    Processor<std::string> p1([&p2]() {
      while (auto message = Receive<std::string>()) {
        p2.Send(std::move(*message));
      }
      p2.Close();
    });

    p1.Send("Hello");
    p1.Send(", ");
    p1.Send("World");
    p1.Send("!");
    p1.Close();

    ASSERT_EQ(output, "Hello, World!");
  }

  SIMPLE_TEST(Tokenize) {
    std::vector<std::string> tokens;

    Processor<std::string> consumer([&]() {
      while (auto token = Receive<std::string>()) {
        tokens.push_back(*token);
      }
    });

    Processor<char> tokenizer([&consumer]() {
      std::string token = "";
      while (auto chr = Receive<char>()) {
        if (*chr == ' ') {
          if (!token.empty()) {
            consumer.Send(token);
            token.clear();
          }
        } else {
          token += *chr;
        }
      }
      if (!token.empty()) {
        consumer.Send(token);
      }
      consumer.Close();
    });

    Processor<std::string> streamer([&tokenizer]() {
      while (auto chunk = Receive<std::string>()) {
        for (auto& chr : *chunk) {
          tokenizer.Send(chr);
        }
      }
      tokenizer.Close();
    });

    streamer.Send("Hel");
    streamer.Send("lo W");
    streamer.Send("orld");
    streamer.Close();

    ASSERT_EQ(tokens.size(), 2);
    ASSERT_EQ(tokens[0], "Hello");
    ASSERT_EQ(tokens[1], "World");
  }
}

RUN_ALL_TESTS()