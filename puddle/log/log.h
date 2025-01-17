#pragma once

#include <chrono>
#include <cstdlib>
#include <string>
#include <unordered_map>

#include "fmt/chrono.h"
#include "fmt/core.h"
#include "puddle/shard/shard.h"

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

// Config configures each logger.
struct Config {
  // Default log level.
  Level level;

  // Overrides the level by logger name.
  std::unordered_map<std::string, Level> overrides;
};

// TODO(andydunstall): Currently uses the default 'FILE*' buffering mode
// (line buffered on stdout, unbuffered on stderr). Instead should use a
// 'fixed' buffering policy, where 'error' logs are immediately flushed, then
// non-error logs are flushed periodically (such as evry second). This
// requires reactor support.
class Logger {
 public:
  Logger(std::string name, FILE* file = stderr);

  std::string name() const { return name_; }

  template <typename... T>
  void Fatal(fmt::format_string<T...> fmt, T&&... args) {
    Log(Level::kFatal, std::move(fmt), std::forward<T>(args)...);
    std::abort();
  }

  template <typename... T>
  void Error(fmt::format_string<T...> fmt, T&&... args) {
    Log(Level::kError, std::move(fmt), std::forward<T>(args)...);
  }

  template <typename... T>
  void Warn(fmt::format_string<T...> fmt, T&&... args) {
    Log(Level::kWarn, std::move(fmt), std::forward<T>(args)...);
  }

  template <typename... T>
  void Info(fmt::format_string<T...> fmt, T&&... args) {
    Log(Level::kInfo, std::move(fmt), std::forward<T>(args)...);
  }

  template <typename... T>
  void Debug(fmt::format_string<T...> fmt, T&&... args) {
    Log(Level::kDebug, std::move(fmt), std::forward<T>(args)...);
  }

  template <typename... T>
  void Trace(fmt::format_string<T...> fmt, T&&... args) {
    Log(Level::kTrace, std::move(fmt), std::forward<T>(args)...);
  }

  template <typename... T>
  void Log(Level level, fmt::format_string<T...> fmt, T&&... args) {
    if (!IsEnabled(level)) {
      return;
    }

    // TODO(andydunstall): Improve performance by:
    // - Use a static buffer to avoid allocations
    // - Cache the formatted time for this second (only format the millisecond)
    // - ...
    // See Seastar util/log.hh.
    //
    auto f = fmt::format(fmt, std::forward<T>(args)...);
    auto now = std::chrono::system_clock::now();
    fmt::print(file_, "{}  {:%Y-%m-%d %T} [{}] [shard {}]  - {}\n",
               LevelToString(level), now, name_, shard::id(), f);
  }

  bool IsEnabled(Level level) const noexcept {
    return __builtin_expect(level <= level_, false);
  }

  void SetLevel(Level level) { level_ = level; }

 private:
  std::string name_;

  FILE* file_;

  Level level_;
};

// Tracks and configures all loggers.
class Registry {
 public:
  void Register(Logger* logger);

  void SetConfig(Config config);

 private:
  Level LoggerLevel(std::string name);

  Config config_;

  std::unordered_map<std::string, Logger*> loggers_;
};

// Global registry that all loggers register with.
Registry* GlobalRegistry();

}  // namespace log
}  // namespace puddle
