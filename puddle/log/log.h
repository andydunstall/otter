#pragma once

#include <chrono>
#include <cstdlib>
#include <string>
#include <unordered_map>

#include "fmt/chrono.h"
#include "fmt/core.h"
#include "puddle/log/level.h"

namespace puddle {
namespace log {

struct Config {
  // Default log level.
  Level level;

  // Overrides the level by logger name.
  std::unordered_map<std::string, Level> overrides;

  static Config Default();
};

// Logger writes logs to the provided file.
//
// Each logger has a name and log level. The name can be used to add level
// overrides to specific loggers.
//
// Each log record includes a level, timestamp, logger name and message.
// Messages are formatted by fmtlib.
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

    // TODO(andydunstall): Rather than use FILE* buffering, buffer within the
    // logger. Some levels shouldn't be buffered (fatal/error), but others can
    // be then flushed periodically.

    auto f = fmt::format(fmt, std::forward<T>(args)...);
    // TODO(andydunstall): Formatting the timestamp for each log is expensive.
    // Instead cache the formatted timestamp second, then only append the
    // millisecond (see Seastar logger).
    auto now = std::chrono::system_clock::now();
    fmt::print(file_, "{}  {:%Y-%m-%d %T} [{}] - {}\n", LevelToString(level),
               now, name_, f);
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

}  // namespace log
}  // namespace puddle
