#pragma once

#include <list>

#include "boost/intrusive/list.hpp"
#include "boost/intrusive/options.hpp"
#include "boost/intrusive/slist.hpp"
#include "puddle/reactor/context.h"

namespace puddle {
namespace reactor {
namespace internal {

// Schedules tasks on the local thread.
class Scheduler {
 public:
  Scheduler();

  bool has_ready() const { return !ready_queue_.empty(); }

  void AddReady(Context* context);

  void AddSleep(Context* context,
                const std::chrono::steady_clock::time_point& sleep_tp);

  Context* Next();

  Context* Terminate(Context* context);

  // Wakes up sleeping contexts whose deadline has passed, and returns the
  // number of nanoseconds until the next contexts deadline, or -1 if there
  // are no sleeping contexts.
  int64_t WakeSleeping();

  void ReleaseTerminated();

 private:
  struct SleepTimeLess {
    bool operator()(const Context& l, const Context& r) const noexcept {
      return l.sleep_time_ < r.sleep_time_;
    }
  };

  using ReadyQueueType = boost::intrusive::list<
      Context,
      boost::intrusive::member_hook<Context, ReadyHook, &Context::ready_hook_>,
      boost::intrusive::constant_time_size<false>>;

  using SleepQueueType = boost::intrusive::multiset<
      Context,
      boost::intrusive::member_hook<Context, SleepHook, &Context::sleep_hook_>,
      boost::intrusive::constant_time_size<false>,
      boost::intrusive::compare<SleepTimeLess>>;

  using TerminateQueueType = boost::intrusive::slist<
      Context,
      boost::intrusive::member_hook<Context, TerminateHook,
                                    &Context::terminated_hook_>,
      boost::intrusive::linear<true>, boost::intrusive::cache_last<true>>;

  ReadyQueueType ready_queue_;

  SleepQueueType sleep_queue_;

  TerminateQueueType terminate_queue_;
};

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
