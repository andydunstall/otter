#pragma once

#include "boost/intrusive_ptr.hpp"
#include "puddle/internal/context.h"

namespace puddle {

// Handle for a user-space thread.
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

  void Detach();

 private:
  // Required for access to task constructor.
  template <typename Fn, typename... Arg>
  friend Task Spawn(Fn&& fn, Arg&&... arg);

  Task(boost::intrusive_ptr<internal::Context> context);

  boost::intrusive_ptr<internal::Context> context_;
};

inline void swap(Task& l, Task& r) noexcept { return l.swap(r); }

}  // namespace puddle
