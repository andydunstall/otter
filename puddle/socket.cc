#include "puddle/socket.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#include <boost/fiber/operations.hpp>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"

namespace puddle {

absl::StatusOr<uint32_t> ParseIPv4(const std::string& s) {
  uint32_t ip;
  if (inet_pton(AF_INET, s.data(), &ip) != 1) {
    return absl::InvalidArgumentError("parse ipv4: invalid address");
  }
  return ip;
}

Socket::Socket(int fd) : fd_{fd} {}

Socket::~Socket() {
  if (fd_ != -1) {
    close(fd_);
  }
}

Socket::Socket(Socket&& o) {
  fd_ = o.fd_;
  // Set to -1 to avoid o closing the socket.
  o.fd_ = -1;
}

Socket& Socket::operator=(Socket&& o) {
  fd_ = o.fd_;
  // Set to -1 to avoid o closing the socket.
  o.fd_ = -1;
  return *this;
}

absl::Status Socket::Listen(const std::string& ip, uint64_t port, int backlog) {
  absl::StatusOr<uint32_t> ipv4 = ParseIPv4(ip);
  if (!ipv4.ok()) {
    return ipv4.status();
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = *ipv4;

  int opt = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    return absl::UnavailableError(
        absl::StrFormat("socket set SO_REUSEADDR: %s", strerror(errno)));
  }

  if (bind(fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    return absl::UnavailableError(
        absl::StrFormat("socket bind: %s", strerror(errno)));
  }
  if (listen(fd_, backlog) < 0) {
    return absl::UnavailableError(
        absl::StrFormat("socket listen: %s", strerror(errno)));
  }

  return absl::OkStatus();
}

Socket Socket::Accept() {
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
    return Socket{fd};
  }
}

Socket Socket::Open() {
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  CHECK_GE(fd, 0);
  return Socket(fd);
}

}  // namespace puddle
