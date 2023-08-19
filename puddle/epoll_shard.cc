#include "puddle/epoll_shard.h"

#include <sys/eventfd.h>

#include "absl/log/check.h"
#include "absl/log/log.h"

namespace puddle {

EpollShard::EpollShard() {
  epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
  CHECK_NE(epoll_fd_, -1);

  wake_fd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  CHECK_GE(wake_fd_, 0);

  RegisterWake(wake_fd_, [fd = wake_fd_]() {
    uint64_t val;
    CHECK_EQ(8, read(fd, &val, sizeof(val)));
  });
}

EpollShard::~EpollShard() {
  if (epoll_fd_ != -1) {
    close(epoll_fd_);
  }
  if (wake_fd_ != -1) {
    close(wake_fd_);
  }
}

EpollShard::EpollShard(EpollShard&& o) {
  epoll_fd_ = o.epoll_fd_;
  wake_fd_ = o.wake_fd_;
  // Set to -1 to avoid o closing the socket.
  o.epoll_fd_ = -1;
  o.wake_fd_ = -1;
}

EpollShard& EpollShard::operator=(EpollShard&& o) {
  epoll_fd_ = o.epoll_fd_;
  wake_fd_ = o.wake_fd_;
  // Set to -1 to avoid o closing the socket.
  o.epoll_fd_ = -1;
  o.wake_fd_ = -1;
  return *this;
}

void EpollShard::Register(int fd, std::function<void()> cb) {
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLIN | EPOLLET;
  event.data.fd = fd;
  CHECK_NE(epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event), -1);
  entries_[fd] = cb;
}

void EpollShard::Wake() {
  uint64_t val = 1;
  CHECK_EQ(8, write(wake_fd_, &val, sizeof(uint64_t)));
}

void EpollShard::Poll(int timeout_ms) {
  int event_count;
  while (true) {
    event_count =
        epoll_wait(epoll_fd_, events_.data(), events_.size(), timeout_ms);
    if (event_count == -1 && errno == EINTR) {
      continue;
    }
    CHECK_NE(event_count, -1);
    break;
  }

  for (int i = 0; i < event_count; i++) {
    entries_[events_[i].data.fd]();
  }
}

void EpollShard::RegisterWake(int fd, std::function<void()> cb) {
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = fd;
  CHECK_NE(epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event), -1);
  entries_[fd] = cb;
}

}  // namespace puddle
