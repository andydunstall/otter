#include "puddle/reactor/scheduler.h"

namespace puddle {
namespace reactor {
namespace internal {

Scheduler::Scheduler() {}

void Scheduler::AddReady(Context* context) { ready_queue_.push_back(*context); }

Context* Scheduler::Next() {
  if (ready_queue_.empty()) {
    return nullptr;
  }
  Context* next = &ready_queue_.front();
  ready_queue_.pop_front();
  return next;
}

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
