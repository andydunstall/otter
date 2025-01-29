#include "puddle/internal/scheduler.h"

namespace puddle {
namespace internal {

void Scheduler::AddReady(Context* context) { ready_queue_.push_back(*context); }

Context* Scheduler::NextReady() {
  if (ready_queue_.empty()) {
    return nullptr;
  }
  Context* next = &ready_queue_.front();
  ready_queue_.pop_front();
  return next;
}

void Scheduler::AddSleep(Context* context,
                         const std::chrono::steady_clock::time_point& tp) {
  context->sleep_tp_ = tp;
  sleep_queue_.insert(*context);
}

std::chrono::steady_clock::time_point Scheduler::NextSleep() {
  auto it = sleep_queue_.begin();
  if (it != sleep_queue_.end()) {
    return it->sleep_tp_;
  }
  return std::chrono::steady_clock::time_point::max();
}

void Scheduler::WakeSleeping() {
  // Wake contexts whose deadline has been reached by moving them to the
  // ready queue.
  //
  // Contexts are sorted by deadline, so once we find a context that hasn't
  // expired we're done.
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  for (SleepQueueType::iterator it = sleep_queue_.begin();
       it != sleep_queue_.end();) {
    Context* c = &(*it);
    if (c->sleep_tp_ <= now) {
      it = sleep_queue_.erase(it);
      ready_queue_.push_back(*c);
    } else {
      return;
    }
  }
}

void Scheduler::AddTerminating(Context* context) {
  terminate_queue_.push_back(*context);
}

void Scheduler::ReleaseTerminating() {
  while (!terminate_queue_.empty()) {
    Context* next = &terminate_queue_.front();
    terminate_queue_.pop_front();

    // As described in intrusive_ptr_release(Context*), we explicitly call
    // release to release the context from the reactor context.
    intrusive_ptr_release(next);
  }
}

}  // namespace internal
}  // namespace puddle
