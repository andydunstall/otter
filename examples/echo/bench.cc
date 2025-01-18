#include "examples/echo/bench.h"

#include "absl/time/clock.h"
#include "puddle/config/config.h"
#include "puddle/net/exception.h"
#include "puddle/net/tcp.h"
#include "puddle/reactor/reactor.h"
#include "puddle/reactor/task.h"

namespace echo {
namespace bench {

void Config::Load(const std::string& path, bool expand_env) {
  if (path == "") {
    return;
  }

  YAML::Node node = puddle::config::LoadYaml(path, expand_env);

  try {
    if (!node.IsMap()) {
      throw puddle::config::Exception{"parse yaml: invalid yaml"};
    }

    if (node["addr"]) {
      if (!node["addr"].IsScalar()) {
        throw puddle::config::Exception{"parse yaml: invalid 'addr'"};
      }
      addr = node["addr"].as<std::string>();
    }

    if (node["requests"]) {
      if (!node["requests"].IsScalar()) {
        throw puddle::config::Exception{"parse yaml: invalid 'requests'"};
      }
      requests = node["requests"].as<uint64_t>();
    }

    if (node["request_size"]) {
      if (!node["request_size"].IsScalar()) {
        throw puddle::config::Exception{"parse yaml: invalid 'request_size'"};
      }
      request_size = node["request_size"].as<uint64_t>();
    }

    if (node["clients"]) {
      if (!node["clients"].IsScalar()) {
        throw puddle::config::Exception{"parse yaml: invalid 'clients'"};
      }
      clients = node["clients"].as<int>();
    }

    if (node["reactor"]) {
      if (!node["reactor"].IsMap()) {
        throw puddle::config::Exception{"parse yaml: invalid 'reactor'"};
      }

      auto reactor_node = node["reactor"];

      if (reactor_node["threads"]) {
        if (!reactor_node["threads"].IsScalar()) {
          throw puddle::config::Exception{"parse yaml: invalid 'threads'"};
        }
        reactor.threads = reactor_node["threads"].as<int>();
      }

      if (reactor_node["cpu_set"]) {
        if (!reactor_node["cpu_set"].IsSequence()) {
          throw puddle::config::Exception{"parse yaml: invalid 'cpu_set'"};
        }
        reactor.cpu_set = reactor_node["cpu_set"].as<std::vector<int>>();
        reactor.threads = reactor.cpu_set.size();
      }

      if (reactor_node["ring_size"]) {
        if (!reactor_node["ring_size"].IsScalar()) {
          throw puddle::config::Exception{"parse yaml: invalid 'ring_size'"};
        }
        reactor.ring_size = reactor_node["ring_size"].as<int>();
      }
    }

    if (node["log"]) {
      if (!node["log"].IsMap()) {
        throw puddle::config::Exception{"parse yaml: invalid 'log'"};
      }

      auto log_node = node["log"];
      if (log_node["level"]) {
        if (!log_node["level"].IsScalar()) {
          throw puddle::config::Exception{"parse yaml: invalid 'level'"};
        }
        puddle::log::Level level =
            puddle::log::LevelFromString(log_node["level"].as<std::string>());
        if (level == static_cast<puddle::log::Level>(-1)) {
          throw puddle::config::Exception{"invalid 'level'"};
        }
        log.level = level;
      }
    }
  } catch (const YAML::Exception& e) {
    throw puddle::config::Exception{"parse yaml: " + e.msg};
  }
}

Config Config::Default() {
  Config config;
  config.addr = "127.0.0.1:4400";
  config.requests = 5'000'000;
  config.request_size = 64;
  config.clients = 100;
  config.reactor = puddle::reactor::Config::Default();
  config.log = puddle::log::Config::Default();
  return config;
}

void Stats::Merge(const Stats& s) { histogram.Merge(s.histogram); }

Benchmark::Benchmark(Config config) : config_{config}, logger_{"bench"} {
  if (config_.clients % config_.reactor.threads != 0) {
    logger_.Warn("threads is not a multiple of clients: {} / {}",
                 config_.clients, config_.reactor.threads);
  }

  shard_config_.resize(config_.reactor.threads);
  for (int i = 0; i != config_.clients; i++) {
    shard_config_[i % config_.reactor.threads].clients++;
  }
  for (uint64_t i = 0; i != config_.requests; i++) {
    shard_config_[i % config_.reactor.threads].requests++;
  }
}

void Benchmark::Shard() {
  Stats stats;

  auto shard_id = puddle::shard::id();
  auto clients = shard_config_[shard_id].clients;
  auto requests = shard_config_[shard_id].requests;

  logger_.Info("shard {}: clients {}; requests {}", shard_id, clients,
               requests);

  std::vector<uint64_t> requests_per_client(clients);
  for (uint64_t i = 0; i != requests; i++) {
    requests_per_client[i % clients]++;
  }

  std::vector<puddle::reactor::Task> tasks;
  for (uint64_t requests : requests_per_client) {
    tasks.push_back(puddle::reactor::local()->Spawn(
        "client", &Benchmark::Client, this, requests, &stats));
  }
  for (auto& task : tasks) {
    task.Join();
  }

  std::lock_guard<std::mutex> lk{mu_};
  stats_.Merge(stats);
}

void Benchmark::Client(uint64_t requests, Stats* stats) {
  puddle::net::TcpConn conn = puddle::net::TcpConn::Connect(config_.addr);

  std::string request(config_.request_size, 'x');
  try {
    for (uint64_t i = 0; i != requests; i++) {
      int64_t start = absl::GetCurrentTimeNanos();

      size_t n_written = 0;
      while (n_written < request.size()) {
        n_written += conn.Write(absl::Span<uint8_t>(
            reinterpret_cast<uint8_t*>(request.data()) + n_written,
            request.size() - n_written));
      }

      size_t n_read = 0;
      while (n_read < request.size()) {
        size_t n = conn.Read(absl::Span<uint8_t>(
            (uint8_t*)request.data() + n_read, request.size() - n_read));
        if (n == 0) {
          throw puddle::net::Exception{"connection closed"};
        }
        n_read -= n;
      }

      int64_t duration_ns = absl::GetCurrentTimeNanos() - start;
      stats->histogram.Add(duration_ns / 1000);  // us
    }
  } catch (const std::exception& e) {
    logger_.Error("benchmark client: {}", e.what());
  }
}

}  // namespace bench
}  // namespace echo
