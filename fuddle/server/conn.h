#pragma once

#include <memory>

#include "puddle/socket.h"

namespace fuddle {
namespace server {

class Conn {
 public:
  Conn(std::unique_ptr<puddle::Socket> socket);

  void ReadLoop();

 private:
  void Ping(absl::Span<uint8_t> b);

  std::unique_ptr<puddle::Socket> socket_;
};

}  // namespace server
}  // namespace fuddle
