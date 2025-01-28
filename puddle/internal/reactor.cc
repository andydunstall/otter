#include "puddle/internal/reactor.h"

#include <cstring>

namespace puddle {
namespace internal {

BlockingRequest::BlockingRequest() : ctx_(Reactor::local()->active_) {}

int BlockingRequest::Wait() {
  // Suspend the current fiber, then the reactor will wake us up once the
  // result is ready.
  Reactor::local()->Suspend();
  return result_;
}

void BlockingRequest::Connect(int sockfd, struct sockaddr* addr,
                              socklen_t addrlen) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&Reactor::local()->ring_);
  io_uring_prep_connect(sqe, sockfd, addr, addrlen);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::Accept(int sockfd, struct sockaddr* addr,
                             socklen_t* addrlen, int flags) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&Reactor::local()->ring_);
  io_uring_prep_accept(sqe, sockfd, addr, addrlen, flags);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::Read(int fd, void* buf, unsigned nbytes, off_t offset) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&Reactor::local()->ring_);
  io_uring_prep_read(sqe, fd, buf, nbytes, offset);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::Write(int fd, const void* buf, unsigned nbytes,
                            off_t offset) {
  struct io_uring_sqe* sqe = io_uring_get_sqe(&Reactor::local()->ring_);
  io_uring_prep_write(sqe, fd, buf, nbytes, offset);
  io_uring_sqe_set_data(sqe, this);
}

void BlockingRequest::SetResult(int result) {
  result_ = result;
  Reactor::local()->Schedule(ctx_);
}

Reactor::Config Reactor::Config::Default() {
  Config config;
  config.ring_size = 1024;
  return config;
}

Reactor::Reactor(Config config) : logger_{"reactor"} {
  int res = io_uring_queue_init(config.ring_size, &ring_, 0);
  if (res != 0) {
    logger_.Fatal("failed to setup io_uring: {}", strerror(-res));
  }
}

Reactor::~Reactor() { io_uring_queue_exit(&ring_); }

void Reactor::Yield() {}

void Reactor::Suspend() {}

void Reactor::Schedule(Context* context) {}

void Reactor::Start(Config config) {
  // Set local reactor.
  local_ = new Reactor{config};
}

thread_local Reactor* Reactor::local_ = nullptr;

}  // namespace internal
}  // namespace puddle
