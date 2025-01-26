#include <chrono>
#include <iostream>

#include "puddle/puddle.h"

using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
  // Start the Puddle runtime.
  puddle::Start();

  puddle::Task t1 = puddle::Spawn([] {
    puddle::SleepFor(500ms);
    std::cout << "task 1" << std::endl;
  });
  puddle::Task t2 = puddle::Spawn([] {
    puddle::SleepFor(2s);
    std::cout << "task 2" << std::endl;
  });
  puddle::Task t3 = puddle::Spawn([] {
    puddle::SleepFor(5s);
    std::cout << "task 3" << std::endl;
  });

  t1.Join();
  t2.Join();
  t3.Join();
}
