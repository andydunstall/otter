#include "puddle/internal/tcp.h"

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <system_error>

#include "puddle/internal/reactor.h"

namespace {

struct sockaddr_in ParseAddr(const std::string& addr) {
  struct sockaddr_in sock_addr;

  memset(&sock_addr, 0, sizeof(sock_addr));
  sock_addr.sin_family = AF_INET;

  size_t pos = addr.find(':');
  if (pos == std::string::npos) {
    throw std::runtime_error("invalid address: " + addr);
  }

  std::string ip_str = addr.substr(0, pos);
  std::string port_str = addr.substr(pos + 1);

  if (ip_str == "") {
    sock_addr.sin_addr.s_addr = INADDR_ANY;
  } else {
    if (inet_pton(AF_INET, ip_str.c_str(), &sock_addr.sin_addr) != 1) {
      throw std::runtime_error("invalid address: " + addr);
    }
  }

  try {
    int port = std::stoi(port_str);
    if (port < 1 || port > 65535) {
      throw std::runtime_error("invalid address: " + addr);
    }
    sock_addr.sin_port = htons(port);
  } catch (const std::exception& e) {
    throw std::runtime_error("invalid address: " + addr);
  }

  return sock_addr;
}

}  // namespace

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
  struct sockaddr_in sock_addr = ParseAddr(addr);

  int opt = 1;
  if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt)) == -1) {
    throw std::system_error(errno, std::system_category(), "socket option");
  }

  if (bind(socket_, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == -1) {
    throw std::system_error(errno, std::system_category(), "socket bind");
  }
}

void TcpSocket::Listen(int backlog) {
  if (listen(socket_, backlog) == -1) {
    throw std::system_error(errno, std::system_category(), "socket listen");
  }
}

TcpSocket TcpSocket::Accept() {
  sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);

  BlockingRequest r;
  r.Accept(socket_, (struct sockaddr*)&client_addr, &addr_len, SOCK_CLOEXEC);
  int conn = r.Wait();
  if (conn < 0) {
    throw std::system_error(-conn, std::system_category(), "socket accept");
  }

  return TcpSocket{conn};
}

void TcpSocket::Connect(const std::string& addr) {
  struct sockaddr_in sock_addr = ParseAddr(addr);

  BlockingRequest r;
  r.Connect(socket_, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
  int res = r.Wait();
  if (res < 0) {
    throw std::system_error(-res, std::system_category(), "socket connect");
  }
}

size_t TcpSocket::Read(uint8_t* buf, size_t size) {
  BlockingRequest r;
  r.Read(socket_, buf, size, 0);
  int read_n = r.Wait();
  if (read_n < 0) {
    throw std::system_error(-read_n, std::system_category(), "socket read");
  }
  return read_n;
}

size_t TcpSocket::Write(const uint8_t* buf, size_t size) {
  BlockingRequest r;
  r.Write(socket_, buf, size, 0);
  int write_n = r.Wait();
  if (write_n == -1) {
    throw std::system_error(-write_n, std::system_category(), "socket write");
  }
  return write_n;
}

TcpSocket TcpSocket::Open() {
  int s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (s == -1) {
    throw std::system_error(errno, std::system_category(), "socket open");
  }
  return TcpSocket{s};
}

}  // namespace internal
}  // namespace puddle
