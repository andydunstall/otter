#pragma once

#include <string_view>

namespace puddle {
namespace log {

// Logger levels.
//
// Levels are in increasing order, so if you log info (3), you'll also log
// fatal (0), error (1) and warn (2).
enum class Level {
  kFatal,
  kError,
  kWarn,
  kInfo,
  kDebug,
  kTrace,
};

Level LevelFromString(std::string_view s);

std::string_view LevelToString(Level level);

}  // namespace log
}  // namespace puddle
