#include "puddle/puddle.h"

#include "puddle/internal/reactor.h"

namespace puddle {

Config Config::Default() {
  Config config;
  config.reactor = internal::Reactor::Config::Default();
  return config;
}

void Start(Config config) { internal::Reactor::Start(config.reactor); }

}  // namespace puddle
