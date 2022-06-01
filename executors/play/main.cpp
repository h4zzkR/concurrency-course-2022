#include <exe/executors/thread_pool.hpp>
#include <exe/executors/strand.hpp>
#include <exe/executors/execute.hpp>

#include <iostream>

using namespace exe::executors;

int main() {

  ThreadPool pool{13};
  size_t counter = 0;
  Strand strand(pool);
  static const size_t kIncrements = 1234;

  for (size_t i = 0; i < kIncrements; ++i) {
    Execute(strand, [&]() {
      ++counter;
    });
  };

  pool.WaitIdle();

  assert(counter == kIncrements);

  pool.Stop();

//  ThreadPool pool{4};
//  Strand strand{pool};
//
//  size_t cs = 0;
//
//  for (size_t i = 0; i < 12345; ++i) {
//    Execute(strand, [&cs]() {
//      ++cs;  // <-- Critical section
//    });
//  }
//
//  pool.WaitIdle();
//
//  std::cout << "# critical sections: " << cs << std::endl;
//
//  pool.Stop();
//  return 0;
}
