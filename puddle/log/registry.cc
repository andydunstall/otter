#include "puddle/log/registry.h"

namespace puddle {
namespace log {

void Registry::Register(Logger* logger) {
  loggers_[logger->name()] = logger;

  logger->SetLevel(LoggerLevel(logger->name()));
}

void Registry::SetConfig(Config config) {
  config_ = config;

  for (const auto& e : loggers_) {
    e.second->SetLevel(LoggerLevel(e.first));
  }
}

Level Registry::LoggerLevel(std::string name) {
  if (config_.overrides.find(name) != config_.overrides.end()) {
    return config_.overrides[name];
  }
  return config_.level;
}

}  // namespace log
}  // namespace puddle
