#include "puddle/server.h"

namespace puddle {

Server::Server(std::shared_ptr<Shard> pool) {}

absl::Status Server::AddListener(const std::string& host, uint16_t port,
                                 std::unique_ptr<Listener> listener) {
  return absl::OkStatus();
}

void Server::Serve() {}

}  // namespace puddle
