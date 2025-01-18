#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "examples/echo/server.h"
#include "fmt/core.h"
#include "puddle/log/log.h"
#include "puddle/reactor/pool.h"

namespace echo {

int Server(const std::vector<std::string>& flags) {
  std::string path = "";
  bool expand_env = false;
  bool help = false;
  for (size_t i = 0; i < flags.size(); i++) {
    if (flags[i] == "-h" || flags[i] == "--help") {
      help = true;
    } else if (flags[i] == "-c" || "--config.path") {
      if (i == flags.size() - 1 || flags[i + 1][0] == '-') {
        fmt::print(
            "error: missing config path: {} (see `echo server --help`)\n",
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

                           This will replaces references to ${{VAR}} or $VAR with the corresponding
                           environment variable. The replacement is case-sensitive.

                           References to undefined variables will be replaced with an empty string. A
                           default value can be given using form ${{VAR:default}}.
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

  puddle::reactor::Pool pool{config.reactor};
  pool.OnAllShards([&] {
    server::Listener listener{};
    listener.Serve(config.addr);
  });

  return EXIT_SUCCESS;
}

int Bench(const std::vector<std::string>& flags) { return EXIT_SUCCESS; }

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
