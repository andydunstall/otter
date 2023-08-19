#include "puddle/server.h"

#include <boost/fiber/operations.hpp>

#include "absl/log/log.h"
#include "puddle/scheduler.h"

namespace puddle {

Server::Server(std::shared_ptr<Shard> shard) : shard_{shard} {}

absl::Status Server::AddListener(const std::string& host, uint16_t port,
                                 std::unique_ptr<Listener> listener) {
  std::unique_ptr<Socket> socket = shard_->OpenSocket();
  absl::Status listen_status = socket->Listen(host, port, 128);
  if (!listen_status.ok()) {
    return listen_status;
  }

  listener->SetListenSocket(shard_, std::move(socket));
  listeners_.push_back(std::move(listener));

  LOG(INFO) << "registered listener; host=" << host << "; port=" << port;

  return absl::OkStatus();
}

void Server::Serve() {
  LOG(INFO) << "starting server";

  boost::fibers::use_scheduling_algorithm<Scheduler>(shard_);

  for (size_t i = 0; i != listeners_.size(); i++) {
    boost::fibers::fiber f([l = std::move(listeners_[i])]() { l->Serve(); });
    listener_fibers_.push_back(std::move(f));
  }

  for (size_t i = 0; i != listener_fibers_.size(); i++) {
    listener_fibers_[i].join();
  }
}

}  // namespace puddle
