#pragma once

#include <chrono>

#include "boost/intrusive/list.hpp"
#include "boost/intrusive/options.hpp"
#include "puddle/internal/context.h"

namespace puddle {
namespace internal {

// Schedules contexts on the local thread.
class Scheduler {
 public:
  // Returns whether there are contexts in the ready queue.
  bool has_ready() const { return !ready_queue_.empty(); }

  // Adds the context to the ready queue.
  void AddReady(Context* context);

  // Takes the next context from the ready queue, or return nullptr if the
  // queue is empty.
  Context* NextReady();

  // Adds the context to the sleep queue. The context will be added to the
  // ready queue when the given time is reached.
  void AddSleep(Context* context,
                const std::chrono::steady_clock::time_point& tp);

  // Returns the time the next context on the sleep queue should be woken up,
  // or time_point::max() if there are no sleeping contexts.
  std::chrono::steady_clock::time_point NextSleep();

  // Adds any contexts in the sleep queue whose deadline has passed to the
  // ready queue.
  void Wake();

 private:
  using ReadyQueueType = boost::intrusive::list<
      Context,
      boost::intrusive::member_hook<Context, ReadyHook, &Context::ready_hook_>,
      boost::intrusive::constant_time_size<false>>;

  struct SleepTpLess {
    bool operator()(const Context& l, const Context& r) const noexcept {
      return l.sleep_tp_ < r.sleep_tp_;
    }
  };

  // Set ordered by next wakeup time.
  using SleepQueueType = boost::intrusive::multiset<
      Context,
      boost::intrusive::member_hook<Context, SleepHook, &Context::sleep_hook_>,
      boost::intrusive::constant_time_size<false>,
      boost::intrusive::compare<SleepTpLess>>;

  ReadyQueueType ready_queue_;

  SleepQueueType sleep_queue_;
};

}  // namespace internal
}  // namespace puddle
