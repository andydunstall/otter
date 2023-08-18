#pragma once

#include <memory>

#include "puddle/conn.h"

namespace puddle {

class Listener {
 public:
  virtual ~Listener() = default;

  // Connection is called when a new connection is accepted.
  virtual void Connection(std::unique_ptr<Conn> conn) = 0;
};

}  // namespace puddle
