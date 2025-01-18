#include "puddle/log/log.h"

namespace puddle {
namespace log {

Level LevelFromString(std::string_view s) {
  if (s == "FATAL") return Level::kFatal;
  if (s == "ERROR") return Level::kError;
  if (s == "WARN") return Level::kWarn;
  if (s == "INFO") return Level::kInfo;
  if (s == "DEBUG") return Level::kDebug;
  if (s == "TRACE") return Level::kTrace;
  return static_cast<Level>(-1);
}

std::string_view LevelToString(Level level) {
  const char* levels[] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
  return levels[static_cast<int>(level)];
}

Config Config::Default() {
  Config config;
  config.level = Level::kInfo;
  return config;
}

Logger::Logger(std::string name, FILE* file) : name_{name}, file_{file} {
  GlobalRegistry()->Register(this);
}

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

Registry* GlobalRegistry() {
  static Registry g_registry;
  return &g_registry;
}

}  // namespace log
}  // namespace puddle
