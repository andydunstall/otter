#include <chrono>
#include <csignal>
#include <mutex>
#include <string>

#include "absl/time/clock.h"
#include "puddle/config/config.h"
#include "puddle/log/log.h"
#include "puddle/net/exception.h"
#include "puddle/net/tcp.h"
#include "puddle/pprof/pprof.h"
#include "puddle/reactor/config.h"
#include "puddle/reactor/pool.h"
#include "puddle/reactor/reactor.h"
#include "puddle/reactor/task.h"
#include "puddle/stats/histogram.h"

namespace echo {

namespace server {

struct Config {
  std::string addr;

  puddle::reactor::Config reactor;

  puddle::log::Config log;

  puddle::pprof::Config pprof;

  void Load(const std::string& path, bool expand_env) {
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

      if (node["pprof"]) {
        if (!node["pprof"].IsMap()) {
          throw puddle::config::Exception{"parse yaml: invalid 'pprof'"};
        }

        auto pprof_node = node["pprof"];
        if (pprof_node["path"]) {
          if (!pprof_node["path"].IsScalar()) {
            throw puddle::config::Exception{"parse yaml: invalid 'path'"};
          }
          pprof.path = pprof_node["path"].as<std::string>();
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

  static Config Default() {
    Config config;
    config.addr = "localhost:4400";
    config.reactor = puddle::reactor::Config::Default();
    config.log = puddle::log::Config::Default();
    return config;
  }
};

class Listener {
 public:
  Listener() : logger_{"echo.listener"} {}

  void Serve(const std::string& addr) {
    auto listener = puddle::net::TcpListener::Bind(addr, 128);
    while (true) {
      auto conn = listener.Accept();
      puddle::reactor::local()
          ->Spawn("conn", &Listener::Conn, this, std::move(conn))
          .Detach();
    }
  }

 private:
  void Conn(puddle::net::TcpConn conn) {
    logger_.Debug("client connected");

    std::array<uint8_t, 256> buf;
    while (true) {
      try {
        size_t read_n = conn.Read(absl::Span<uint8_t>(buf));
        if (read_n == 0) {
          logger_.Debug("client disconnected");
          return;
        }

        while (read_n > 0) {
          read_n -= conn.Write(absl::Span<uint8_t>(buf.data(), read_n));
        }
      } catch (const std::exception& e) {
        logger_.Warn("client: {}", e.what());
      }
    }
  }

  puddle::log::Logger logger_;
};

}  // namespace server

namespace bench {

struct Config {
  std::string addr;

  uint64_t requests;

  uint64_t request_size;

  int clients;

  puddle::reactor::Config reactor;

  puddle::pprof::Config pprof;

  puddle::log::Config log;

  void Load(const std::string& path, bool expand_env) {
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

      if (node["pprof"]) {
        if (!node["pprof"].IsMap()) {
          throw puddle::config::Exception{"parse yaml: invalid 'pprof'"};
        }

        auto pprof_node = node["pprof"];
        if (pprof_node["path"]) {
          if (!pprof_node["path"].IsScalar()) {
            throw puddle::config::Exception{"parse yaml: invalid 'path'"};
          }
          pprof.path = pprof_node["path"].as<std::string>();
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

  static Config Default() {
    Config config;
    config.addr = "127.0.0.1:4400";
    config.requests = 5'000'000;
    config.request_size = 64;
    config.clients = 100;
    config.reactor = puddle::reactor::Config::Default();
    config.log = puddle::log::Config::Default();
    return config;
  }
};

struct Stats {
  puddle::stats::Histogram histogram;

  void Merge(const Stats& s) { histogram.Merge(s.histogram); }
};

class Benchmark {
 public:
  Benchmark(Config config) : config_{config}, logger_{"echo.bench"} {
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

  Stats stats() const { return stats_; };

  void Shard() {
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

 private:
  struct ShardConfig {
    uint64_t requests;
    int clients;
  };

  void Client(uint64_t requests, Stats* stats) {
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

  Stats stats_;

  std::vector<ShardConfig> shard_config_;

  Config config_;

  std::mutex mu_;

  puddle::log::Logger logger_;
};

}  // namespace bench

void SignalHandler(int signal) {
  puddle::pprof::Stop();
  exit(EXIT_SUCCESS);
}

int Server(const std::vector<std::string>& flags) {
  // TODO(andydunstall): Not yet handling signals properly, though for now
  // catch SIGINT/SIGNTERM to flush the profiler.
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);

  std::string path = "";
  bool expand_env = false;
  bool help = false;
  for (size_t i = 0; i < flags.size(); i++) {
    if (flags[i] == "-h" || flags[i] == "--help") {
      help = true;
    } else if (flags[i] == "-c" || flags[i] == "--config.path") {
      if (i == flags.size() - 1 || flags[i + 1][0] == '-') {
        fmt::print(
            "error: missing config path: {} (kee `echo server --help`)\n",
            flags[i]);
        return EXIT_FAILURE;
      }
      path = flags[i + 1];
      i++;
    } else if (flags[i] == "-e" || "--config.expand-env") {
      expand_env = true;
    } else {
      fmt::print("error: unknown flag: {} (see `echo server --help`)\n",
                 flags[i]);
      return EXIT_FAILURE;
    }
  }

  if (help) {
    fmt::print(R"(Echo server.

The server loads configuration from YAML. Configure a YAML file using
`--config.path`. When enabling '--config.expand-env', the server will expand
environment variables in the loaded YAML configuration.
  )");

    fmt::print(R"(
Examples:
  # Start a echo server with default configuration.
  echo server

  # Load configuration from YAML.
  echo server --config.path ./server.yaml

  # Load configuration from YAML and expand environment variables.
  echo server --config.path ./server.yaml --config.expand-env
)");

    fmt::print(R"(
Flags:
  -h, --help               Dispay command information
  -c, --config.path
                           YAML config path.
  -e, --config.expand-env
                           Whether to expand environment variables in the config file.

                           This will replaces references to ${VAR} or $VAR with the corresponding
                           environment variable. The replacement is case-sensitive.

                           References to undefined variables will be replaced with an empty string.
)");
    return EXIT_SUCCESS;
  }

  server::Config config = server::Config::Default();
  try {
    config.Load(path, expand_env);
  } catch (const std::exception& e) {
    fmt::print("failed to load config: {}\n", e.what());
    return EXIT_FAILURE;
  }

  puddle::log::GlobalRegistry()->SetConfig(config.log);

  puddle::log::Logger logger{"main"};
  logger.Info("starting echo server; addr = {}", config.addr);

  if (config.pprof.path != "") {
    logger.Info("starting pprof profile; path = {}", config.pprof.path);
    puddle::pprof::Start(config.pprof);
  }

  puddle::reactor::Pool pool{config.reactor};
  pool.OnAllShards([&] {
    server::Listener listener{};
    listener.Serve(config.addr);
  });

  return EXIT_SUCCESS;
}

int Bench(const std::vector<std::string>& flags) {
  std::string path = "";
  bool expand_env = false;
  bool help = false;
  for (size_t i = 0; i < flags.size(); i++) {
    if (flags[i] == "-h" || flags[i] == "--help") {
      help = true;
    } else if (flags[i] == "-c" || flags[i] == "--config.path") {
      if (i == flags.size() - 1 || flags[i + 1][0] == '-') {
        fmt::print(
            "error: missing config path: {} (see `neptune server --help`)\n",
            flags[i]);
        return EXIT_FAILURE;
      }
      path = flags[i + 1];
      i++;
    } else if (flags[i] == "-e" || "--config.expand-env") {
      expand_env = true;
    } else {
      fmt::print("error: unknown flag: {} (see `neptune server --help`)\n",
                 flags[i]);
      return EXIT_FAILURE;
    }
  }

  if (help) {
    fmt::print(R"(Neptune benchmark tool.

The benchmark loads configuration from YAML. Configure a YAML file using
`--config.path`. When enabling '--config.expand-env', Neptune will expand
environment variables in the loaded YAML configuration.
  )");

    fmt::print(R"(
Examples:
  # Start a Neptune benchmark with default configuration.
  neptune bench

  # Load configuration from YAML.
  neptune bench --config.path ./bench.yaml

  # Load configuration from YAML and expand environment variables.
  neptune server --config.path ./bench.yaml --config.expand-env
)");

    fmt::print(R"(
Flags:
  -h, --help               Dispay command information
  -c, --config.path
                           YAML config path.
  -e, --config.expand-env
                           Whether to expand environment variables in the config file.

                           This will replaces references to ${VAR} or $VAR with the corresponding
                           environment variable. The replacement is case-sensitive.

                           References to undefined variables will be replaced with an empty string.
)");
    return EXIT_SUCCESS;
  }

  bench::Config config = bench::Config::Default();
  try {
    config.Load(path, expand_env);
  } catch (const std::exception& e) {
    fmt::print("failed to load config: {}\n", e.what());
    return EXIT_FAILURE;
  }

  puddle::log::GlobalRegistry()->SetConfig(config.log);

  puddle::log::Logger logger{"main"};
  logger.Info("starting echo bench; addr = {}", config.addr);

  if (config.pprof.path != "") {
    logger.Info("starting pprof profile; path = {}", config.pprof.path);
    puddle::pprof::Start(config.pprof);
  }

  bench::Benchmark bench{config};

  puddle::reactor::Pool pool{config.reactor};

  auto start = std::chrono::high_resolution_clock::now();
  pool.OnAllShards([&bench] { bench.Shard(); });
  auto end = std::chrono::high_resolution_clock::now();

  puddle::pprof::Stop();

  auto stats = bench.stats();
  auto duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
  auto milliseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
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

  return EXIT_SUCCESS;
}

}  // namespace echo

int main(int argc, char* argv[]) {
  using main_func_type = std::function<int(const std::vector<std::string>&)>;

  struct command {
    std::string_view name;
    main_func_type func;
    std::string_view description;
  };
  const command commands[] = {
      {"server", echo::Server, "echo server"},
      {"bench", echo::Bench, "echo benchmark"},
  };

  std::string command_name;
  if (argc >= 2) {
    command_name = argv[1];

    // Don't consider flags in the command name.
    if (command_name[0] == '-') {
      command_name = "";
    }
  }
  std::vector<std::string> flags;
  for (int i = (command_name.empty()) ? 1 : 2; i < argc; i++) {
    flags.emplace_back(argv[i]);
  }

  for (const command& command : commands) {
    if (command.name == command_name) {
      return command.func(flags);
    }
  }

  if (command_name != "") {
    fmt::print("error: unknown command: {} (see `echo --help`)\n",
               command_name);
    return EXIT_FAILURE;
  }

  for (const std::string& flag : flags) {
    if (flag[0] != '-') {
      fmt::print("error: invalid flag: {}\n", flag);
      return EXIT_FAILURE;
    }

    if (flag.size() > 2 && (flag[1] != '-' || flag.size() <= 4)) {
      // Long hand flags must be prefixed by '--'.
      fmt::print("error: invalid flag: {}\n", flag);
      return EXIT_FAILURE;
    }

    if (flag != "-h" && flag != "--help") {
      fmt::print("error: unknown flag: {} (see `echo --help`)\n", flag);
      return EXIT_FAILURE;
    }
  }

  fmt::print(R"(Puddle echo server and benchmark.

Start the echo server with:

  $ echo server

Or benchmark the echo server with:

  $ echo bench
)");

  fmt::print("\nAvailable Commands:\n");
  for (const command& command : commands) {
    std::string padding(8 - command.name.size(), ' ');
    fmt::print("  {}:{}{}\n", command.name, padding, command.description);
  }

  fmt::print(R"(
Flags:
  -h, --help      Dispay command information
)");

  fmt::print(R"(
Use `echo [command] --help` for more information about a command.
)");
}
