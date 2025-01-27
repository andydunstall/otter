#include <benchmark/benchmark.h>

#include "puddle/log/log.h"

namespace {

// Opens /dev/null.
//
// Logger benchmarks write to /dev/null to discard the output, though still
// make the same syscalls as writing to stdout/stderr.
FILE* OpenDevNull() {
  FILE* f = fopen("/dev/null", "w");
  if (!f) {
    throw std::runtime_error(std::string("open /dev/null: ") + strerror(errno));
  }
  return f;
}

}  // namespace

puddle::log::Logger logger{""};

static void DoSetup(const benchmark::State& state) {
  logger = puddle::log::Logger{"bench", OpenDevNull()};
}

template <typename... T>
static void BM_Logger(benchmark::State& state, fmt::format_string<T...> fmt,
                      T&&... args) {
  for (auto _ : state) {
    logger.Error(fmt, std::forward<T>(args)...);
  }
}

template <typename... T>
static void BM_LoggerDisabled(benchmark::State& state,
                              fmt::format_string<T...> fmt, T&&... args) {
  for (auto _ : state) {
    // Trace is disabled for the logger.
    logger.Trace(fmt, std::forward<T>(args)...);
  }
}

BENCHMARK_CAPTURE(BM_Logger, small_string, "my-log")->Setup(DoSetup);
BENCHMARK_CAPTURE(BM_Logger, long_string,
                  "my-log my-log my-log my-log my-log my-log my-log my-log "
                  "my-log my-log my-log my-log my-log my-log my-log my-log "
                  "my-log my-log my-log my-log my-log my-log my-log my-log")
    ->Setup(DoSetup);
BENCHMARK_CAPTURE(BM_Logger, formatted_string, "my-log {} {} {} {} {}", "arg-1",
                  11111, "arg-2", 22222, "arg-3", 33333)
    ->Setup(DoSetup);

// Logs to the disabled logger should have negligible overhead.
BENCHMARK_CAPTURE(BM_LoggerDisabled, formatted_string, "my-log {} {} {} {} {}",
                  "arg-1", 11111, "arg-2", 22222, "arg-3", 33333)
    ->Setup(DoSetup);

BENCHMARK_MAIN();
