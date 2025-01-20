#pragma once

#include <functional>

#include "puddle/log/log.h"
#include "puddle/reactor/config.h"

namespace puddle {
namespace reactor {

// Manages a pool of reactors.
class Pool {
 public:
  Pool(Config config);

  // Sets up a reactor on the configured threads and runs the given function
  // on each.
  void OnAllShards(std::function<void()> f);

 private:
  Config config_;

  log::Logger logger_;
};

}  // namespace reactor
}  // namespace puddle
