#pragma once

#include <boost/fiber/fiber.hpp>
#include <memory>

#include "absl/status/status.h"
#include "puddle/listener.h"
#include "puddle/shard.h"

namespace puddle {

class Server {
 public:
  Server(std::shared_ptr<Shard> shard);

  absl::Status AddListener(const std::string& host, uint16_t port,
                           std::unique_ptr<Listener> listener);

  void Serve();

 private:
  std::vector<std::unique_ptr<Listener>> listeners_;

  std::vector<boost::fibers::fiber> listener_fibers_;

  std::shared_ptr<Shard> shard_;
};

}  // namespace puddle
