#include "puddle/listener.h"

#include <boost/fiber/context.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/scheduler.hpp>

#include "absl/log/check.h"
#include "absl/log/log.h"

namespace puddle {

void Listener::Serve() {
  CHECK(socket_) << "listen socket not set";

  while (true) {
    std::unique_ptr<Socket> conn = socket_->Accept();

    boost::fibers::fiber([s = std::move(conn), this]() mutable {
      Connection(std::move(s));
    }).detach();
  }
}

void Listener::SetListenSocket(std::shared_ptr<Shard> shard,
                               std::unique_ptr<Socket> socket) {
  shard_ = shard;
  socket_ = std::move(socket);
}

}  // namespace puddle
