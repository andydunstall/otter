#pragma once

#include <liburing.h>
#include <netinet/in.h>

#include "boost/intrusive_ptr.hpp"
#include "puddle/internal/context.h"
#include "puddle/log/log.h"

namespace puddle {
namespace internal {

// BlockingRequest requests a operation from the reactor (io_uring), then
// blocks the current context until the result is ready.
//
// This is similar to a future/promise, except it is not thread safe.
class BlockingRequest {
 public:
  BlockingRequest();

  int Wait();

  void Connect(int sockfd, struct sockaddr* addr, socklen_t addrlen);

  void Accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen, int flags);

  void Read(int fd, void* buf, unsigned nbytes, off_t offset);

  void Write(int fd, const void* buf, unsigned nbytes, off_t offset);

  void SetResult(int result);

 private:
  internal::Context* ctx_;

  int result_;
};

// Reactor manages scheduling tasks and asynchronous IO.
class Reactor {
 public:
  struct Config {
    // io_uring ring size.
    int ring_size;

    static Config Default();
  };

  Reactor(Config config);

  ~Reactor();

  Reactor(const Reactor&) = delete;
  Reactor& operator=(const Reactor&) = delete;

  Reactor(Reactor&&) = delete;
  Reactor& operator=(Reactor&&) = delete;

  // Returns the active context.
  Context* active() { return active_; }

  // Spawn a task context.
  template <typename Fn, typename... Arg>
  boost::intrusive_ptr<Context> Spawn(Fn&& fn, Arg&&... arg) {
    return nullptr;
  }

  void Yield();

  void Suspend();

  // Schedule adds the context to the ready queue.
  void Schedule(Context* context);

  // Returns the reactor in the local thread.
  static Reactor* local() { return local_; }

  // Starts the reactor in the local thread.
  static void Start(Config config);

 private:
  // Required for access to ring_.
  friend BlockingRequest;

  static thread_local Reactor* local_;

  Context* active_;

  io_uring ring_;

  log::Logger logger_;
};

}  // namespace internal
}  // namespace puddle
