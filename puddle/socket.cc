#include "puddle/socket.h"

#include <arpa/inet.h>
#include <sys/socket.h>

#include "absl/strings/str_format.h"

namespace puddle {

Socket::Socket(int fd) : fd_{fd} {}

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

absl::StatusOr<uint32_t> ParseIPv4(const std::string& s) {
  uint32_t ip;
  if (inet_pton(AF_INET, s.data(), &ip) != 1) {
    return absl::InvalidArgumentError("parse ipv4: invalid address");
  }
  return ip;
}

}  // namespace puddle
