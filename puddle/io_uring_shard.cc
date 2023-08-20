#include "puddle/io_uring_shard.h"

#include "absl/log/check.h"
#include "absl/log/log.h"

namespace puddle {

constexpr unsigned kRingSize = 256;

IoUringShard::IoUringShard() {
  ring_ = std::make_unique<struct io_uring>();
  CHECK_EQ(io_uring_queue_init(kRingSize, ring_.get(), 0), 0);
}

IoUringShard::~IoUringShard() {
  if (ring_) {
    io_uring_queue_exit(ring_.get());
  }
}

IoUringShard::IoUringShard(IoUringShard&& o) {
  ring_ = std::move(o.ring_);
  o.ring_ = nullptr;
}

IoUringShard& IoUringShard::operator=(IoUringShard&& o) {
  ring_ = std::move(o.ring_);
  o.ring_ = nullptr;
  return *this;
}

std::unique_ptr<Socket> IoUringShard::OpenSocket() {
  return std::make_unique<IoUringSocket>(IoUringSocket::Open(ring_.get()));
}

void IoUringShard::Register(int fd, std::function<void()> cb) {
  // TODO(andydunstall)
}

void IoUringShard::Wake() {
  // TODO(andydunstall)
}

void IoUringShard::Poll(int timeout_ms) {
  io_uring_submit(ring_.get());

  // TODO(andydunstall) For now only handling block forever or don't block.

  if (timeout_ms == -1) {
    // TODO(andydunstall) Block
  }

  uint32_t cqe_count = 0;
  unsigned ring_head;
  struct io_uring_cqe* cqe;
  io_uring_for_each_cqe(ring_.get(), ring_head, cqe) {
    cqe_count++;

    LOG(INFO) << "poll cqe";

    // TODO(andydunstall) dispatch
  }
  if (cqe_count) {
    io_uring_cq_advance(ring_.get(), cqe_count);
  }
}

}  // namespace puddle
