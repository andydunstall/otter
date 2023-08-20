#include "puddle/io_uring_socket.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#include <liburing.h>

#include <boost/fiber/operations.hpp>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"

namespace puddle {

IoUringSocket::IoUringSocket(int fd, struct io_uring* ring) : Socket{fd}, ring_{ring} {}

IoUringSocket::~IoUringSocket() {
  if (fd_ != -1) {
    close(fd_);
  }
}

IoUringSocket::IoUringSocket(IoUringSocket&& o) {
  fd_ = o.fd_;
  ring_ = o.ring_;
  // Set to -1 to avoid o closing the socket.
  o.fd_ = -1;
  o.ring_ = nullptr;
}

IoUringSocket& IoUringSocket::operator=(IoUringSocket&& o) {
  fd_ = o.fd_;
  ring_ = o.ring_;
  // Set to -1 to avoid o closing the socket.
  o.fd_ = -1;
  o.ring_ = nullptr;
  return *this;
}

std::unique_ptr<Socket> IoUringSocket::Accept() {
  sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);

  int fd;
  while (true) {
    fd = accept4(fd_, (struct sockaddr*)&client_addr, &addr_len,
                 SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (fd == -1) {
      if (errno == EWOULDBLOCK) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(ring_);
        io_uring_prep_accept(sqe, fd_, (struct sockaddr *)&client_addr,
                             &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
        boost::fibers::context::active()->suspend();
        continue;
      }
      if (errno == EINTR) {
        continue;
      }
      CHECK_NE(fd, -1);
    }
    return std::make_unique<IoUringSocket>(fd);
  }
}

absl::StatusOr<size_t> IoUringSocket::Read(Buffer* buf) {
  // TODO(andydunstall)
  return 0;
}

absl::StatusOr<size_t> IoUringSocket::Write(const absl::Span<uint8_t>& buf) {
  // TODO(andydunstall)
  return 0;
}

IoUringSocket IoUringSocket::Open(struct io_uring* ring) {
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  CHECK_GE(fd, 0);
  return IoUringSocket(fd, ring);
}

}  // namespace puddle
