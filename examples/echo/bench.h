#pragma once

#include <chrono>
#include <mutex>

#include "puddle/log/log.h"
#include "puddle/reactor/config.h"
#include "puddle/stats/histogram.h"

namespace echo {
namespace bench {

struct Config {
  std::string addr;

  uint64_t requests;

  uint64_t request_size;

  int clients;

  puddle::reactor::Config reactor;

  puddle::log::Config log;

  void Load(const std::string& path, bool expand_env);

  static Config Default();
};

struct Stats {
  puddle::stats::Histogram histogram;

  void Merge(const Stats& s);
};

class Benchmark {
 public:
  Benchmark(Config config);

  Stats stats() const { return stats_; };

  void Shard();

 private:
  struct ShardConfig {
    uint64_t requests;
    int clients;
  };

  void Client(uint64_t requests, Stats* stats);

  Stats stats_;

  std::vector<ShardConfig> shard_config_;

  Config config_;

  std::mutex mu_;

  puddle::log::Logger logger_;
};

}  // namespace bench
}  // namespace echo
