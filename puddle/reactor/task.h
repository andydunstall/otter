#pragma once

#include "puddle/reactor/context.h"

namespace puddle {
namespace reactor {

class Reactor;

// A user space thread scheduled by a local reactor.
class Task {
 public:
  Task() = default;

  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;

  Task(Task&& task) noexcept : context_{} { swap(task); }

  Task& operator=(Task&& task) noexcept {
    context_.swap(task.context_);
    return *this;
  }

  void swap(Task& task) noexcept { context_.swap(task.context_); }

  void Join();

  void Detach() {}

 private:
  friend Reactor;

  Task(boost::intrusive_ptr<internal::Context> context) : context_(context) {}

  boost::intrusive_ptr<internal::Context> context_;
};

inline void swap(Task& l, Task& r) noexcept { return l.swap(r); }

}  // namespace reactor
}  // namespace puddle
