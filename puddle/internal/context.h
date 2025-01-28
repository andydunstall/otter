#pragma once

#include <chrono>

#include "boost/intrusive/list.hpp"
#include "boost/intrusive/set.hpp"
#include "boost/intrusive_ptr.hpp"

namespace puddle {
namespace internal {

class Scheduler;

using ReadyHook = boost::intrusive::list_member_hook<
    boost::intrusive::link_mode<boost::intrusive::safe_link>>;

using SleepHook = boost::intrusive::set_member_hook<
    boost::intrusive::link_mode<boost::intrusive::safe_link>>;

// Context represents the a tasks execution state.
class Context {
 public:
  friend void intrusive_ptr_add_ref(Context* c) noexcept;
  friend void intrusive_ptr_release(Context* c) noexcept;

 private:
  // Required to access intrusive member hooks.
  friend Scheduler;

  ReadyHook ready_hook_;

  SleepHook sleep_hook_;

  // Time the context is asleep till when it is in the schedulers sleep queue.
  std::chrono::steady_clock::time_point sleep_tp_;
};

inline void intrusive_ptr_add_ref(Context* c) noexcept {
  // TODO(andydunstall)
}

inline void intrusive_ptr_release(Context* c) noexcept {
  // TODO(andydunstall)
}

}  // namespace internal
}  // namespace puddle
