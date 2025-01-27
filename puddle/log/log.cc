#include "puddle/log/log.h"

#include "puddle/log/registry.h"

namespace puddle {
namespace log {

Config Config::Default() {
  Config config;
  config.level = Level::kInfo;
  return config;
}

Logger::Logger(std::string name, FILE* file) : name_{name}, file_{file} {
  // Register to set this loggers level based on the logger config.
  Registry::global()->Register(this);
}

}  // namespace log
}  // namespace puddle
