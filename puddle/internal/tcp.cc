#include "puddle/internal/tcp.h"

#include <unistd.h>

namespace puddle {
namespace internal {

TcpSocket::TcpSocket(int socket) : socket_{socket} {}

TcpSocket::~TcpSocket() {
  if (socket_ != -1) {
    close(socket_);
  }
}

TcpSocket::TcpSocket(TcpSocket&& s) {
  socket_ = s.socket_;
  s.socket_ = -1;
}

TcpSocket& TcpSocket::operator=(TcpSocket&& s) {
  socket_ = s.socket_;
  s.socket_ = -1;
  return *this;
}

void TcpSocket::Bind(const std::string& addr) {
  // TODO(andydunstall)
}

void TcpSocket::Listen(int backlog) {
  // TODO(andydunstall)
}

TcpSocket TcpSocket::Accept() {
  // TODO(andydunstall)
  return TcpSocket{};
}

void TcpSocket::Connect(const std::string& addr) {
  // TODO(andydunstall)
}

size_t TcpSocket::Read(uint8_t* buf, size_t size) {
  // TODO(andydunstall)
  return 0;
}

size_t TcpSocket::Write(const uint8_t* buf, size_t size) {
  // TODO(andydunstall)
  return size;
}

TcpSocket TcpSocket::Open() {
  // TODO(andydunstall)
  return TcpSocket{};
}

}  // namespace internal
}  // namespace puddle
