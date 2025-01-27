#include "puddle/log/level.h"

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

}  // namespace log
}  // namespace puddle
