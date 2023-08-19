#include "puddle/listener.h"

#include <boost/fiber/context.hpp>
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
    socket_->Accept();

    LOG(INFO) << "conn accepted";
  }
}

void Listener::SetListenSocket(std::shared_ptr<Shard> shard,
                               std::unique_ptr<Socket> socket) {
  shard_ = shard;
  socket_ = std::move(socket);
}

}  // namespace puddle
