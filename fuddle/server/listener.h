#pragma once

#include "puddle/listener.h"

namespace fuddle {
namespace server {

class Listener : public puddle::Listener {
 public:
  void Connection(std::unique_ptr<puddle::Socket> conn) override;

 private:
  void Ping(absl::Span<uint8_t> b, puddle::Socket* conn);
};

}  // namespace server
}  // namespace fuddle
