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

Context* Scheduler::Terminate(Context* context) {
  terminate_queue_.push_back(*context);
  return Next();
}

void Scheduler::ReleaseTerminated() {
  while (!terminate_queue_.empty()) {
    Context* next = &terminate_queue_.front();
    terminate_queue_.pop_front();

    intrusive_ptr_release(next);
  }
}

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
