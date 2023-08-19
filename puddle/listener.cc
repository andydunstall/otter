#include "puddle/listener.h"

#include <boost/fiber/context.hpp>
#include <boost/fiber/fiber.hpp>
#include <boost/fiber/scheduler.hpp>

#include "absl/log/check.h"
#include "absl/log/log.h"

namespace puddle {

void Listener::Serve() {
  CHECK(socket_) << "listen socket not set";

  boost::fibers::context* c = boost::fibers::context::active();
  shard_->Register(socket_->fd(), [c]() {
    boost::fibers::context::active()->get_scheduler()->schedule(c);
  });

  while (true) {
    Socket conn = socket_->Accept();

    LOG(INFO) << "conn accepted";

    boost::fibers::fiber([s = std::move(conn), this]() mutable {
      // Register for IO events.
      boost::fibers::context* c = boost::fibers::context::active();
      shard_->Register(s.fd(), [c]() {
        boost::fibers::context::active()->get_scheduler()->schedule(c);
      });
      boost::fibers::context::active()->suspend();

      Connection(std::move(s));

      // TODO(andydunstall) unregister from shard.
    }).detach();
  }
}

void Listener::SetListenSocket(std::shared_ptr<Shard> shard,
                               std::unique_ptr<Socket> socket) {
  shard_ = shard;
  socket_ = std::move(socket);
}

}  // namespace puddle
