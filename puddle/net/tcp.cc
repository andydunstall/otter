#include "puddle/net/tcp.h"

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include "puddle/net/exception.h"
#include "puddle/reactor/reactor.h"

namespace puddle {
namespace net {

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
  // TODO(andydunstall): Use addr.

  int opt = 1;
  if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)) == -1) {
    throw Exception{"socket option", errno};
  }

  struct sockaddr_in sock_addr;
  sock_addr.sin_family = AF_INET;
  sock_addr.sin_addr.s_addr = INADDR_ANY;
  sock_addr.sin_port = htons(4400);

  if (bind(socket_, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1) {
    throw Exception{"socket bind", errno};
  }
}

void TcpSocket::Listen(int backlog) {
  if (listen(socket_, backlog) == -1) {
    throw Exception{"socket listen", errno};
  }
}

TcpSocket TcpSocket::Accept() {
  sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);

  reactor::BlockingRequest r;
  r.PrepAccept(socket_, (struct sockaddr*)&client_addr, &addr_len,
               SOCK_CLOEXEC);
  int conn = r.Wait();
  if (conn < 0) {
    throw Exception{"socket accept", -conn};
  }

  return TcpSocket{conn};
}

void TcpSocket::Connect(const std::string& addr) {
  // TODO(andydunstall): Use addr.

  struct sockaddr_in sock_addr;
  sock_addr.sin_family = AF_INET;
  inet_pton(AF_INET, "127.0.0.1", &sock_addr.sin_addr);
  sock_addr.sin_port = htons(4400);

  reactor::BlockingRequest r;
  r.PrepConnect(socket_, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
  int res = r.Wait();
  if (res < 0) {
    throw Exception{"socket connect", -res};
  }
}

size_t TcpSocket::Read(absl::Span<uint8_t> buf) {
  reactor::BlockingRequest r;
  r.PrepRead(socket_, buf.data(), buf.size(), 0);
  int read_n = r.Wait();
  if (read_n < 0) {
    throw Exception{"socket read", -read_n};
  }
  return read_n;
}

size_t TcpSocket::Write(const absl::Span<uint8_t>& buf) {
  reactor::BlockingRequest r;
  r.PrepWrite(socket_, buf.data(), buf.size(), 0);
  int write_n = r.Wait();
  if (write_n == -1) {
    throw Exception{"socket write", -write_n};
  }
  return write_n;
}

TcpSocket TcpSocket::Open() {
  int s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (s == -1) {
    throw Exception{"socket open", errno};
  }
  return TcpSocket{s};
}

TcpConn::TcpConn(TcpSocket socket) : socket_{std::move(socket)} {}

TcpConn::TcpConn(TcpConn&& c) { socket_ = std::move(c.socket_); }

TcpConn& TcpConn::operator=(TcpConn&& c) {
  socket_ = std::move(c.socket_);
  return *this;
}

size_t TcpConn::Read(absl::Span<uint8_t> buf) { return socket_.Read(buf); }

size_t TcpConn::Write(const absl::Span<uint8_t>& buf) {
  return socket_.Write(buf);
}

TcpConn TcpConn::Connect(const std::string& addr) {
  TcpSocket socket = TcpSocket::Open();
  socket.Connect(addr);
  return TcpConn{std::move(socket)};
}

TcpConn TcpListener::Accept() {
  TcpSocket socket = socket_.Accept();
  return TcpConn{std::move(socket)};
}

TcpListener::TcpListener(TcpListener&& l) { socket_ = std::move(l.socket_); }

TcpListener& TcpListener::operator=(TcpListener&& l) {
  socket_ = std::move(l.socket_);
  return *this;
}

TcpListener TcpListener::Bind(const std::string& addr, int backlog) {
  TcpSocket socket = TcpSocket::Open();
  socket.Bind(addr);
  socket.Listen(backlog);
  return TcpListener{std::move(socket)};
}

TcpListener::TcpListener(TcpSocket s) : socket_(std::move(s)) {}

}  // namespace net
}  // namespace puddle
