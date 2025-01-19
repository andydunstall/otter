#pragma once

#include <list>

#include "boost/intrusive/list.hpp"
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

  Context* Next();

  Context* Terminate(Context* context);

  void ReleaseTerminated();

 private:
  using ReadyQueueType = boost::intrusive::list<
      Context,
      boost::intrusive::member_hook<Context, ReadyHook, &Context::ready_hook_>,
      boost::intrusive::constant_time_size<false>>;

  using TerminateQueueType = boost::intrusive::slist<
      Context,
      boost::intrusive::member_hook<Context, TerminateHook,
                                    &Context::terminated_hook_>,
      boost::intrusive::linear<true>, boost::intrusive::cache_last<true>>;

  ReadyQueueType ready_queue_;

  TerminateQueueType terminate_queue_;
};

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
