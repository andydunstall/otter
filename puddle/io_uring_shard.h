#pragma once

#include <liburing.h>

#include <memory>

#include "puddle/io_uring_socket.h"
#include "puddle/shard.h"

namespace puddle {

class IoUringShard : public Shard {
 public:
  IoUringShard();

  ~IoUringShard();

  IoUringShard(const IoUringShard&) = delete;
  IoUringShard& operator=(const IoUringShard&) = delete;

  IoUringShard(IoUringShard&&);
  IoUringShard& operator=(IoUringShard&&);

  std::unique_ptr<Socket> OpenSocket() override;

  void Register(int fd, std::function<void()> cb) override;

  void Wake() override;

  void Poll(int timeout_ms) override;

 private:
  std::unique_ptr<struct io_uring> ring_;
};

}  // namespace puddle
