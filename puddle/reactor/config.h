#pragma once

#include <cstddef>
#include <vector>

namespace puddle {
namespace reactor {

struct Config {
  // Number of reactor threads.
  int threads;

  // CPU set to run reactor threads on. Overrides `threads`.
  std::vector<int> cpu_set;

  // io_uring ring size.
  int ring_size;

  static Config Default();
};

}  // namespace reactor
}  // namespace puddle
