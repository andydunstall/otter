#pragma once

#include <sys/epoll.h>

#include <array>
#include <functional>
#include <unordered_map>

#include "puddle/shard.h"

namespace puddle {

class EpollShard : public Shard {
 public:
  EpollShard();

  ~EpollShard();

  EpollShard(const EpollShard&) = delete;
  EpollShard& operator=(const EpollShard&) = delete;

  EpollShard(EpollShard&&);
  EpollShard& operator=(EpollShard&&);

  void Register(int fd, std::function<void()> cb) override;

  void Wake() override;

  void Poll(int timeout_ms) override;

 private:
  void RegisterWake(int fd, std::function<void()> cb);

  std::array<struct epoll_event, 128> events_;

  std::unordered_map<int, std::function<void()>> entries_;

  int epoll_fd_;

  int wake_fd_;
};

}  // namespace puddle
