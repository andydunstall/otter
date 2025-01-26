#pragma once

#include <string>

namespace puddle {
namespace net {

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
};

}  // namespace net
}  // namespace puddle
