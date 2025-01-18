#pragma once

#include <string>

#include "puddle/log/log.h"
#include "puddle/net/tcp.h"
#include "puddle/pprof/pprof.h"
#include "puddle/reactor/config.h"

namespace echo {
namespace server {

struct Config {
  std::string addr;

  puddle::reactor::Config reactor;

  puddle::log::Config log;

  puddle::pprof::Config pprof;

  void Load(const std::string& path, bool expand_env);

  static Config Default();
};

class Listener {
 public:
  Listener();

  void Serve(const std::string& addr);

 private:
  void Conn(puddle::net::TcpConn conn);

  puddle::log::Logger logger_;
};

}  // namespace server
}  // namespace echo
