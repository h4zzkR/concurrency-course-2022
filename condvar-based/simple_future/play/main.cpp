#include <futures/promise.hpp>

#include <iostream>
#include <thread>

int main() {
  stdlike::Promise<std::string> p;
  auto f = p.MakeFuture();

  p.SetValue("Hi");
  std::cout << f.Get() << std::endl;

  return 0;
}
