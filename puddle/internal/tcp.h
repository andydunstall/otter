#pragma once

#include <string>

namespace puddle {
namespace internal {

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

  size_t Read(uint8_t* buf, size_t size);

  size_t Write(const uint8_t* buf, size_t size);

  static TcpSocket Open();

 private:
  int socket_ = -1;
};

}  // namespace internal
}  // namespace puddle
