#pragma once

#include <liburing.h>
#include <netinet/in.h>

#include "boost/intrusive_ptr.hpp"
#include "puddle/internal/context.h"
#include "puddle/internal/scheduler.h"
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
//
// The reactor always has an "active" context, which is currently running.
// When the active context yields, the scheduler selects another context to
// run.
//
// The reactor has it's own context which is always ready to run, which manages
// submitting and waiting for events with io_uring. The reactor context will
// block waiting for I/O if there are no other ready contexts.
//
// To perform I/O, contexts submit io_uring operations to the reactor using
// BlockingRequest, then suspend their context. The once the operation has
// complete, the reactor adds the context that scheduled the operation to
// the ready queue so it can be scheduled.
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
    auto context = internal::TaskContext<Fn, Arg...>::Spawn(
        std::forward<Fn>(fn), std::forward<Arg>(arg)...);
    scheduler_.AddReady(context.get());
    return context;
  }

  // Yield the current context so the scheduler can switch to another context.
  // The current context will be added to the schedulers ready queue to be
  // scheduled again.
  void Yield();

  // Suspend the current context, which is the same as yield except the current
  // context is not added to the schedulers ready queue. It will have to be
  // awoken by another context to run again.
  void Suspend();

  // Schedule adds the context to the ready queue.
  void Schedule(Context* context);

  // Dispatch submits events to io_uring and dispatches completed operations to
  // other tasks.
  void Dispatch();

  // Returns the reactor in the local thread.
  static Reactor* local() { return local_; }

  // Starts the reactor in the local thread.
  static void Start(Config config);

 private:
  // Required for access to ring_.
  friend BlockingRequest;

  static thread_local Reactor* local_;

  Scheduler scheduler_;

  // Active context thats currently running.
  Context* active_;

  // Main context that started the reactor.
  Context main_context_;

  boost::intrusive_ptr<internal::Context> reactor_context_;

  io_uring ring_;

  log::Logger logger_;
};

}  // namespace internal
}  // namespace puddle
