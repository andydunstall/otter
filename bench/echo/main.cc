#include "bench/echo/bench.h"

int main(int argc, char* argv[]) {
  echo::Config config;
  config.addr = "127.0.0.1:4411";
  config.requests = 1'000'000;
  config.request_size = 64;
  config.clients = 10;
  config.reactor = puddle::Config::Default();

  // Start the Puddle runtime.
  puddle::Start(config.reactor);

  puddle::log::Logger logger{"main"};
  logger.Info("starting benchmark");

  echo::Benchmark bench{config};
  bench.Run();

  auto stats = bench.stats();
  auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(stats.duration)
          .count();
  auto request_per_ms = config.requests / milliseconds;

  fmt::println(
      R"(
  Requests per second: {}

  Latency (us):
    Min: {}
    p50: {}
    p99: {}
    p99.9: {}
    p99.99: {}
    Max: {}
    Std dev: {:.2f}
)",
      request_per_ms * 1000, stats.histogram.min(),
      stats.histogram.Percentile(50.0), stats.histogram.Percentile(99.0),
      stats.histogram.Percentile(99.9), stats.histogram.Percentile(99.99),
      stats.histogram.max(), stats.histogram.StdDev());
}
