#include "puddle/internal/sync.h"

#include "puddle/internal/context.h"

namespace puddle {
namespace internal {

void WaitQueue::NotifyOne() {
  if (queue_.empty()) {
    return;
  }

  Context* next = queue_.front();
  queue_.pop_front();
  next->Schedule();
}

void WaitQueue::NotifyAll() {
  while (!queue_.empty()) {
    Context* next = queue_.front();
    queue_.pop_front();
    next->Schedule();
  }
}

void WaitQueue::SuspendAndWait(Context* c) {
  queue_.push_back(c);
  c->Suspend();
}

}  // namespace internal
}  // namespace puddle
