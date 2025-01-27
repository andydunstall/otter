#include "puddle/puddle.h"

#include "puddle/internal/reactor.h"
#include "puddle/log/registry.h"

namespace puddle {

Config Config::Default() {
  Config config;
  config.log = log::Config::Default();
  config.reactor = internal::Reactor::Config::Default();
  return config;
}

void Start(Config config) {
  log::Registry::global()->SetConfig(config.log);
  internal::Reactor::Start(config.reactor);
}

}  // namespace puddle
