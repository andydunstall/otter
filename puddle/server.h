#pragma once

#include <memory>

#include "absl/status/status.h"
#include "puddle/listener.h"
#include "puddle/shard.h"

namespace puddle {

class Server {
 public:
  Server(std::shared_ptr<Shard> pool);

  absl::Status AddListener(const std::string& host, uint16_t port,
                           std::unique_ptr<Listener> listener);

  void Serve();

 private:
  std::vector<std::unique_ptr<Listener>> listeners_;
};

}  // namespace puddle
