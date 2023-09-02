#pragma once

#include <functional>
#include <memory>

#include "puddle/socket.h"

namespace puddle {

class Shard {
 public:
  virtual ~Shard() = default;

  virtual std::unique_ptr<Socket> OpenSocket() = 0;

  virtual void Wake() = 0;

  virtual void Poll(int timeout_ms) = 0;
};

}  // namespace puddle
