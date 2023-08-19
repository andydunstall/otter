#pragma once

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace puddle {

class Shard;

class Socket {
 public:
  Socket(int fd = -1);

  ~Socket();

  int fd() const { return fd_; }

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  Socket(Socket&&);
  Socket& operator=(Socket&&);

  absl::Status Listen(const std::string& ip, uint64_t port, int backlog);

  Socket Accept();

  static Socket Open();

 private:
  int fd_;
};

}  // namespace puddle
