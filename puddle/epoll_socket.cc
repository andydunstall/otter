#include "puddle/epoll_socket.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#include <boost/fiber/operations.hpp>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"

namespace puddle {

EpollSocket::EpollSocket(int fd) : Socket{fd} {}

EpollSocket::~EpollSocket() {
  if (fd_ != -1) {
    close(fd_);
  }
}

EpollSocket::EpollSocket(EpollSocket&& o) {
  fd_ = o.fd_;
  // Set to -1 to avoid o closing the socket.
  o.fd_ = -1;
}

EpollSocket& EpollSocket::operator=(EpollSocket&& o) {
  fd_ = o.fd_;
  // Set to -1 to avoid o closing the socket.
  o.fd_ = -1;
  return *this;
}

std::unique_ptr<Socket> EpollSocket::Accept() {
  sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);

  int fd;
  while (true) {
    fd = accept4(fd_, (struct sockaddr*)&client_addr, &addr_len,
                 SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (fd == -1) {
      if (errno == EWOULDBLOCK) {
        boost::fibers::context::active()->suspend();
        continue;
      }
      if (errno == EINTR) {
        continue;
      }
      CHECK_NE(fd, -1);
    }
    return std::make_unique<EpollSocket>(fd);
  }
}

absl::StatusOr<size_t> EpollSocket::Read(Buffer* buf) {
  while (true) {
    auto write_buf = buf->write_buf();
    const ssize_t read_n = read(fd_, write_buf.data(), write_buf.size());
    if (read_n == -1) {
      if (errno == EWOULDBLOCK) {
        boost::fibers::context::active()->suspend();
        continue;
      }
      if (errno == EINTR) {
        continue;
      }
      return absl::UnavailableError(
          absl::StrFormat("socket read: %s", strerror(errno)));
    }
    if (read_n == 0) {
      return absl::CancelledError(absl::StrFormat("socket closed"));
    }
    return read_n;
  }
}

absl::StatusOr<size_t> EpollSocket::Write(const absl::Span<uint8_t>& buf) {
  while (true) {
    const ssize_t write_n = write(fd_, buf.data(), buf.size());
    if (write_n == -1) {
      if (errno == EWOULDBLOCK) {
        boost::fibers::context::active()->suspend();
        continue;
      }
      if (errno == EINTR) {
        continue;
      }
      return absl::UnavailableError(
          absl::StrFormat("socket write: %s", strerror(errno)));
    }
    return write_n;
  }
}

EpollSocket EpollSocket::Open() {
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  CHECK_GE(fd, 0);
  return EpollSocket(fd);
}

}  // namespace puddle
