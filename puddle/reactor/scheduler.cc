#include "puddle/reactor/scheduler.h"

namespace puddle {
namespace reactor {
namespace internal {

Scheduler::Scheduler() {}

void Scheduler::AddReady(Context* context) { ready_queue_.push_back(*context); }

void Scheduler::AddSleep(
    Context* context, const std::chrono::steady_clock::time_point& sleep_time) {
  context->sleep_time_ = sleep_time;
  sleep_queue_.insert(*context);
}

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

int64_t Scheduler::WakeSleeping() {
  // Wake contexts whose deadline has been reached by moving them to the
  // ready queue.
  //
  // Contexts are sorted by deadline, so once we find a context that hasn't
  // expired we're done.
  std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
  for (SleepQueueType::iterator it = sleep_queue_.begin();
       it != sleep_queue_.end();) {
    Context* c = &(*it);
    if (c->sleep_time_ <= now) {
      it = sleep_queue_.erase(it);
      ready_queue_.push_back(*c);
    } else {
      it = sleep_queue_.begin();
      if (it != sleep_queue_.end()) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                   it->sleep_time_ - now)
            .count();
      }
      return -1;
    }
  }

  return -1;
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
