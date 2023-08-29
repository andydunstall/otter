#include "fuddle/server/listener.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "fuddle/server/protocol.h"

namespace fuddle {
namespace server {

void Listener::Connection(std::unique_ptr<puddle::Socket> socket) {
  Conn conn{std::move(socket)};
  conn.ReadLoop();
}

}  // namespace server
}  // namespace fuddle
