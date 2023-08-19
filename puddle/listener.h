#pragma once

#include <memory>

#include "puddle/shard.h"
#include "puddle/socket.h"

namespace puddle {

class Server;

class Listener {
 public:
  virtual ~Listener() = default;

  // Connection is called when a new connection is accepted.
  virtual void Connection(Socket&& s) = 0;

 private:
  friend Server;

  void Serve();

  void SetListenSocket(std::shared_ptr<Shard> shard,
                       std::unique_ptr<Socket> socket);

  std::shared_ptr<Shard> shard_;

  std::unique_ptr<Socket> socket_;
};

}  // namespace puddle
