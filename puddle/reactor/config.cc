#include "puddle/reactor/config.h"

#include <sched.h>

namespace puddle {
namespace reactor {

Config Config::Default() {
  cpu_set_t cpus;
  CPU_ZERO(&cpus);
  sched_getaffinity(0, sizeof(cpus), &cpus);

  Config config;
  // Default to using all available threads.
  config.threads = CPU_COUNT(&cpus);
  config.ring_size = 1024;
  return config;
}

}  // namespace reactor
}  // namespace puddle
