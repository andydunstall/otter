#pragma once

#include <string>
#include <unordered_map>

#include "puddle/log/log.h"

namespace puddle {
namespace log {

// Tracks and configures all loggers.
class Registry {
 public:
  void Register(Logger* logger);

  void SetConfig(Config config);

  // Returns the global registry, used to configure all loggers.
  static Registry* global() {
    static Registry registry;
    return &registry;
  }

 private:
  Level LoggerLevel(std::string name);

  Config config_;

  std::unordered_map<std::string, Logger*> loggers_;
};

}  // namespace log
}  // namespace puddle
