#include "puddle/reactor/wait_queue.h"

#include "puddle/reactor/context.h"

namespace puddle {
namespace reactor {
namespace internal {

void WaitQueue::NotifyOne() {
  if (queue_.empty()) {
    return;
  }

  Context* next = queue_.front();
  queue_.pop_front();
  next->Wake();
}

void WaitQueue::NotifyAll() {
  while (!queue_.empty()) {
    Context* next = queue_.front();
    queue_.pop_front();
    next->Wake();
  }
}

void WaitQueue::SuspendAndWait(Context* c) {
  queue_.push_back(c);
  c->Suspend();
}

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
