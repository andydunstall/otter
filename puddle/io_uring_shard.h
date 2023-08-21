#pragma once

#include <liburing.h>

#include <boost/fiber/future/promise.hpp>
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
  friend IoUringSocket;

  std::unique_ptr<boost::fibers::promise<int>> RequestAccept(
      int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags);

  std::unique_ptr<struct io_uring> ring_;
};

}  // namespace puddle
