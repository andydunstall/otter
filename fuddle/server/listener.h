#pragma once

#include "fuddle/server/conn.h"
#include "puddle/listener.h"

namespace fuddle {
namespace server {

class Listener : public puddle::Listener {
 public:
  void Connection(std::unique_ptr<puddle::Socket> socket) override;
};

}  // namespace server
}  // namespace fuddle
