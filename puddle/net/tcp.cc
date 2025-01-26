#include "puddle/net/tcp.h"

namespace puddle {
namespace net {

TcpConn::TcpConn(TcpConn&& c) { socket_ = std::move(c.socket_); }

TcpConn& TcpConn::operator=(TcpConn&& c) {
  socket_ = std::move(c.socket_);
  return *this;
}

size_t TcpConn::Read(uint8_t* buf, size_t size) {
  return socket_.Read(buf, size);
}

size_t TcpConn::Write(const uint8_t* buf, size_t size) {
  return socket_.Write(buf, size);
}

TcpConn TcpConn::Connect(const std::string& addr) {
  internal::TcpSocket socket = internal::TcpSocket::Open();
  socket.Connect(addr);
  return TcpConn{std::move(socket)};
}

TcpConn::TcpConn(internal::TcpSocket socket) : socket_{std::move(socket)} {}

TcpListener::TcpListener(TcpListener&& l) { socket_ = std::move(l.socket_); }

TcpListener& TcpListener::operator=(TcpListener&& l) {
  socket_ = std::move(l.socket_);
  return *this;
}

TcpConn TcpListener::Accept() {
  internal::TcpSocket socket = socket_.Accept();
  return TcpConn{std::move(socket)};
}

TcpListener TcpListener::Bind(const std::string& addr, int backlog) {
  internal::TcpSocket socket = internal::TcpSocket::Open();
  socket.Bind(addr);
  socket.Listen(backlog);
  return TcpListener{std::move(socket)};
}

TcpListener::TcpListener(internal::TcpSocket s) : socket_(std::move(s)) {}

}  // namespace net
}  // namespace puddle
