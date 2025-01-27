#pragma once

#include <chrono>

#include "puddle/internal/reactor.h"
#include "puddle/task.h"

namespace puddle {

struct Config {
  internal::Reactor::Config reactor;

  static Config Default();
};

// Start the puddle runtime.
void Start(Config config = Config::Default());

// Spawn a task (user-space thread).
template <typename Fn, typename... Arg>
Task Spawn(Fn&& fn, Arg&&... arg) {
  return Task{};
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
void SleepUntil(const std::chrono::time_point<Clock, Duration>& time) {}

template <typename Rep, typename Period>
void SleepFor(const std::chrono::duration<Rep, Period>& duration) {}

}  // namespace puddle
