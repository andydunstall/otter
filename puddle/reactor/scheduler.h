#pragma once

#include "boost/intrusive/list.hpp"
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

 private:
  using ReadyQueueType = boost::intrusive::list<
      Context,
      boost::intrusive::member_hook<Context, ReadyHook, &Context::ready_hook_>,
      boost::intrusive::constant_time_size<false>>;

  ReadyQueueType ready_queue_{};
};

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
