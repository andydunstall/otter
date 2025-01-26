#pragma once

#include <string>

#include "puddle/internal/tcp.h"

namespace puddle {
namespace net {

class TcpListener;

class TcpConn {
 public:
  TcpConn() = default;

  TcpConn(const TcpConn& c) = delete;
  TcpConn& operator=(const TcpConn& c) = delete;

  TcpConn(TcpConn&& c);
  TcpConn& operator=(TcpConn&& c);

  size_t Read(uint8_t* buf, size_t size);

  size_t Write(const uint8_t* buf, size_t size);

  static TcpConn Connect(const std::string& addr);

 private:
  friend TcpListener;

  TcpConn(internal::TcpSocket socket);

  internal::TcpSocket socket_;
};

class TcpListener {
 public:
  TcpListener() = default;

  TcpListener(const TcpListener& l) = delete;
  TcpListener& operator=(const TcpListener& l) = delete;

  TcpListener(TcpListener&& l);
  TcpListener& operator=(TcpListener&& l);

  TcpConn Accept();

  static TcpListener Bind(const std::string& addr, int backlog);

 private:
  TcpListener(internal::TcpSocket s);

  internal::TcpSocket socket_;
};

}  // namespace net
}  // namespace puddle
