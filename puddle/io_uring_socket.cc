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

  std::unique_ptr<boost::fibers::promise<int>> promise = shard_->RequestAccept(
      fd_, (struct sockaddr*)&client_addr, &addr_len, SOCK_CLOEXEC);
  boost::fibers::future<int> future = promise->get_future();
  future.wait();
  int fd = future.get();
  CHECK_NE(fd, -1);

  return std::make_unique<IoUringSocket>(fd, shard_);
}

absl::Status IoUringSocket::Connect(const std::string& ip, uint64_t port) {
  absl::StatusOr<uint32_t> ipv4 = ParseIPv4(ip);
  if (!ipv4.ok()) {
    return ipv4.status();
  }

  sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = *ipv4;
  socklen_t addr_len = sizeof(server_addr);

  std::unique_ptr<boost::fibers::promise<int>> promise =
      shard_->RequestConnect(fd_, (struct sockaddr*)&server_addr, addr_len);
  boost::fibers::future<int> future = promise->get_future();
  future.wait();
  int res = future.get();
  if (res < 0) {
    return absl::UnavailableError(
        absl::StrFormat("socket connect: %s", strerror(-res)));
  }
  return absl::OkStatus();
}

absl::StatusOr<size_t> IoUringSocket::Read(Buffer* buf) {
  auto write_buf = buf->write_buf();

  std::unique_ptr<boost::fibers::promise<int>> promise =
      shard_->RequestRead(fd_, write_buf.data(), write_buf.size(), 0);
  boost::fibers::future<int> future = promise->get_future();
  future.wait();
  ssize_t read_n = future.get();
  if (read_n < 0) {
    return absl::UnavailableError(
        absl::StrFormat("socket read: %s", strerror(-read_n)));
  }
  if (read_n == 0) {
    return absl::UnavailableError("socket closed");
  }
  return read_n;
}

absl::StatusOr<size_t> IoUringSocket::Write(const absl::Span<uint8_t>& buf) {
  std::unique_ptr<boost::fibers::promise<int>> promise =
      shard_->RequestWrite(fd_, buf.data(), buf.size(), 0);
  boost::fibers::future<int> future = promise->get_future();
  future.wait();
  ssize_t write_n = future.get();
  if (write_n == -1) {
    return absl::UnavailableError("socket write");
  }
  return write_n;
}

IoUringSocket IoUringSocket::Open(IoUringShard* shard) {
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
  CHECK_GE(fd, 0);
  return IoUringSocket(fd, shard);
}

}  // namespace puddle
