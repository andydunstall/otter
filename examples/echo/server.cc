#include "examples/echo/server.h"

#include "puddle/config/config.h"
#include "puddle/log/log.h"
#include "puddle/reactor/config.h"
#include "puddle/reactor/reactor.h"

namespace echo {
namespace server {

Config Config::Default() {
  Config config;
  config.addr = "localhost:4400";
  config.reactor = puddle::reactor::Config::Default();
  config.log = puddle::log::Config::Default();
  return config;
}

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

Listener::Listener() : logger_{"echo.listener"} {}

void Listener::Serve(const std::string& addr) {
  auto listener = puddle::net::TcpListener::Bind(addr, 128);
  while (true) {
    auto conn = listener.Accept();
    puddle::reactor::local()
        ->Spawn("conn", &Listener::Conn, this, std::move(conn))
        .Detach();
  }
}

void Listener::Conn(puddle::net::TcpConn conn) {
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

}  // namespace server
}  // namespace echo
