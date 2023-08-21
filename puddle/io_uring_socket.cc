#include "puddle/io_uring_socket.h"

#include <arpa/inet.h>
#include <liburing.h>
#include <sys/socket.h>

#include <boost/fiber/operations.hpp>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "puddle/io_uring_shard.h"

namespace puddle {

IoUringSocket::IoUringSocket(int fd, IoUringShard* shard)
    : Socket{fd}, shard_{shard} {}

IoUringSocket::~IoUringSocket() {
  if (fd_ != -1) {
    close(fd_);
  }
}

IoUringSocket::IoUringSocket(IoUringSocket&& o) {
  fd_ = o.fd_;
  shard_ = o.shard_;
  // Set to -1 to avoid o closing the socket.
  o.fd_ = -1;
}

IoUringSocket& IoUringSocket::operator=(IoUringSocket&& o) {
  fd_ = o.fd_;
  shard_ = o.shard_;
  // Set to -1 to avoid o closing the socket.
  o.fd_ = -1;
  return *this;
}

std::unique_ptr<Socket> IoUringSocket::Accept() {
  sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);

  std::unique_ptr<boost::fibers::promise<int>> promise =
      shard_->RequestAccept(fd_, (struct sockaddr*)&client_addr, &addr_len,
                            SOCK_NONBLOCK | SOCK_CLOEXEC);
  boost::fibers::future<int> future = promise->get_future();
  future.wait();
  int fd = future.get();
  CHECK_NE(fd, -1);

  return std::make_unique<IoUringSocket>(fd, shard_);
}

absl::StatusOr<size_t> IoUringSocket::Read(Buffer* buf) {
  // TODO(andydunstall)
  return 0;
}

absl::StatusOr<size_t> IoUringSocket::Write(const absl::Span<uint8_t>& buf) {
  // TODO(andydunstall)
  return 0;
}

IoUringSocket IoUringSocket::Open(IoUringShard* shard) {
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  CHECK_GE(fd, 0);
  return IoUringSocket(fd, shard);
}

}  // namespace puddle
