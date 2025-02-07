#pragma once

#include <string>

#include "puddle/log/log.h"
#include "puddle/puddle.h"
#include "puddle/stats/histogram.h"

namespace echo {

struct Config {
  std::string addr;

  uint64_t requests;

  uint64_t request_size;

  int clients;

  puddle::Config reactor;
};

struct Stats {
  puddle::stats::Histogram histogram;

  std::chrono::nanoseconds duration;
};

class Benchmark {
 public:
  Benchmark(Config config);

  Stats stats() const { return stats_; };

  void Run();

 private:
  void Client(uint64_t requests);

  Stats stats_;

  Config config_;

  puddle::log::Logger logger_;
};

}  // namespace echo
