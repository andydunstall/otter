#pragma once

#include "absl/types/span.h"

namespace puddle {
namespace net {

class TcpSocket {
 public:
  TcpSocket(int socket = -1);

  ~TcpSocket();

  TcpSocket(const TcpSocket& s) = delete;
  TcpSocket& operator=(const TcpSocket& s) = delete;

  TcpSocket(TcpSocket&& s);
  TcpSocket& operator=(TcpSocket&& s);

  void Bind(const std::string& addr);

  void Listen(int backlog);

  TcpSocket Accept();

  void Connect(const std::string& addr);

  size_t Read(absl::Span<uint8_t> buf);

  size_t Write(const absl::Span<uint8_t>& buf);

  static TcpSocket Open();

 private:
  int socket_ = -1;
};

class TcpConn {
 public:
  TcpConn() = default;

  TcpConn(TcpSocket socket);

  TcpConn(const TcpConn& c) = delete;
  TcpConn& operator=(const TcpConn& c) = delete;

  TcpConn(TcpConn&& c);
  TcpConn& operator=(TcpConn&& c);

  size_t Read(absl::Span<uint8_t> buf);

  size_t Write(const absl::Span<uint8_t>& buf);

  static TcpConn Connect(const std::string& addr);

 private:
  TcpSocket socket_;
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
  TcpListener(TcpSocket s);

  TcpSocket socket_;
};

}  // namespace net
}  // namespace puddle
