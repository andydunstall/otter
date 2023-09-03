#include "otter/server/listener.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "otter/server/protocol.h"

namespace otter {
namespace server {

Listener::Listener(std::shared_ptr<storage::Storage> storage)
    : storage_{storage} {}

void Listener::Connection(std::unique_ptr<puddle::Socket> socket) {
  Conn conn{std::move(socket), storage_};
  conn.ReadLoop();
}

}  // namespace server
}  // namespace otter
