#pragma once

#include <memory>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "puddle/buffer.h"
#include "puddle/reader.h"
#include "puddle/writer.h"

namespace puddle {

class Shard;

class Socket : public Reader, public Writer {
 public:
  Socket(int fd = -1);

  virtual ~Socket() = default;

  int fd() const { return fd_; }

  absl::Status Listen(const std::string& ip, uint64_t port, int backlog);

  virtual std::unique_ptr<Socket> Accept() = 0;

  virtual absl::Status Connect(const std::string& ip, uint64_t port) = 0;

 protected:
  int fd_;
};

absl::StatusOr<uint32_t> ParseIPv4(const std::string& s);

}  // namespace puddle
