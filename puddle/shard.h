#pragma once

#include <functional>
#include <memory>

#include "puddle/socket.h"

namespace puddle {

class Shard {
 public:
  virtual ~Shard() = default;

  std::unique_ptr<Socket> OpenSocket();

  virtual void Register(int fd, std::function<void()> cb) = 0;

  virtual void Wake() = 0;

  virtual void Poll(int timeout_ms) = 0;
};

}  // namespace puddle
