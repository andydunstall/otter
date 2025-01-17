#pragma once

#include <list>

#include "puddle/reactor/context.h"

namespace puddle {
namespace reactor {
namespace internal {

// Schedules tasks on the local thread.
class Scheduler {
 public:
  Scheduler();

  size_t ready() const { return ready_queue_.size(); }

  void AddReady(Context* context);

  Context* Next();

 private:
  std::list<Context*> ready_queue_;
};

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
