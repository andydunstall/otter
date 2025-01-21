#pragma once

#include <liburing.h>

#include "puddle/log/log.h"
#include "puddle/reactor/config.h"
#include "puddle/reactor/context.h"
#include "puddle/reactor/scheduler.h"
#include "puddle/reactor/task.h"

namespace puddle {
namespace reactor {

class BlockingRequest {
 public:
  BlockingRequest();

  int Wait();

  void PrepConnect(int sockfd, struct sockaddr* addr, socklen_t addrlen);

  void PrepAccept(int sockfd, struct sockaddr* addr, socklen_t* addrlen,
                  int flags);

  void PrepRead(int fd, void* buf, unsigned nbytes, off_t offset);

  void PrepWrite(int fd, const void* buf, unsigned nbytes, off_t offset);

  void SetResult(int result);

 private:
  internal::Context* ctx_;

  int result_;
};

class ReactorContext;

// Reactor manages scheduling tasks and asynchronous IO.
//
// Each thread has it's own local reactor and scheduler, which can be
// accessed with reactor::local().
class Reactor {
 public:
  Reactor(Config config);

  ~Reactor();

  Reactor(const Reactor&) = delete;
  Reactor& operator=(const Reactor&) = delete;

  Reactor(Reactor&&) = delete;
  Reactor& operator=(Reactor&&) = delete;

  internal::Context* active() { return active_; }

  internal::Scheduler* scheduler() { return &scheduler_; }

  template <typename Fn, typename... Arg>
  Task Spawn(const std::string& name, Fn&& fn, Arg&&... arg) {
    auto context = internal::TaskContext<Fn, Arg...>::Allocate(
        name, std::forward<Fn>(fn), std::forward<Arg>(arg)...);
    scheduler_.AddReady(context.get());
    return Task{context};
  }

  // Yield the current task so the scheduler can switch to another task. The
  // current task will be added to the schedulers ready queue to be scheduled
  // again.
  void Yield();

  // Suspend the current task, which is the same as yield except the current
  // task is not added to the schedulers ready queue. It will have to be awoken
  // by another task to run again.
  void Suspend();

  template <typename Clock, typename Duration>
  void SleepUntil(const std::chrono::time_point<Clock, Duration>& time) {
    scheduler_.AddSleep(active_, time);
    Suspend();
  }

  template <typename Rep, typename Period>
  void SleepFor(const std::chrono::duration<Rep, Period>& duration) {
    SleepUntil(std::chrono::steady_clock::now() + duration);
  }

  boost::context::fiber Terminate();

  void Dispatch();

  static Reactor* local() { return local_; }

  static void Setup(Config config);

 private:
  friend BlockingRequest;

  void DispatchEvents();

  static thread_local Reactor* local_;

  internal::Scheduler scheduler_;

  io_uring ring_;

  internal::Context* active_;

  internal::Context main_context_;

  boost::intrusive_ptr<internal::Context> reactor_context_;

  log::Logger logger_;
};

// Returns the threads local reactor.
Reactor* local();

}  // namespace reactor
}  // namespace puddle
