#include "bench/echo/bench.h"

#include <exception>

#include "absl/time/clock.h"
#include "puddle/net/tcp.h"

namespace echo {

Benchmark::Benchmark(Config config) : config_{config}, logger_{"bench"} {}

void Benchmark::Run() {
  auto start = std::chrono::high_resolution_clock::now();

  std::vector<uint64_t> requests_per_client(config_.clients);
  for (uint64_t i = 0; i != config_.requests; i++) {
    requests_per_client[i % config_.clients]++;
  }

  std::vector<puddle::Task> tasks;
  for (uint64_t requests : requests_per_client) {
    tasks.push_back(puddle::Spawn(&Benchmark::Client, this, requests));
  }
  for (auto& task : tasks) {
    task.Join();
  }

  auto end = std::chrono::high_resolution_clock::now();
  stats_.duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
}

void Benchmark::Client(uint64_t requests) {
  puddle::net::TcpConn conn = puddle::net::TcpConn::Connect(config_.addr);

  std::string request(config_.request_size, 'x');
  try {
    for (uint64_t i = 0; i != requests; i++) {
      int64_t start = absl::GetCurrentTimeNanos();

      size_t n_written = 0;
      while (n_written < request.size()) {
        n_written +=
            conn.Write(reinterpret_cast<uint8_t*>(request.data() + n_written),
                       request.size() - n_written);
      }

      size_t n_read = 0;
      while (n_read < request.size()) {
        size_t n =
            conn.Read(reinterpret_cast<uint8_t*>(request.data() + n_read),
                      request.size() - n_read);
        if (n == 0) {
          throw std::runtime_error{"connection closed"};
        }
        n_read -= n;
      }

      int64_t duration_ns = absl::GetCurrentTimeNanos() - start;
      stats_.histogram.Add(duration_ns / 1000);  // us
    }
  } catch (const std::exception& e) {
    logger_.Error("benchmark client: {}", e.what());
  }
}

}  // namespace echo
