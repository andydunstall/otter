#include "puddle/internal/context.h"

#include "puddle/internal/reactor.h"

namespace puddle {
namespace internal {

Context::Context() : ref_count_{1} {}

Context::~Context() {
  assert(!ready_hook_.is_linked());
  assert(!sleep_hook_.is_linked());
}

void Context::Join() {
  if (terminated_) return;

  join_queue_.SuspendAndWait(Reactor::local()->active());
}

void Context::Suspend() { Reactor::local()->Suspend(); }

void Context::Schedule() { Reactor::local()->Schedule(this); }

boost::context::fiber Context::Terminate() {
  terminated_ = true;
  join_queue_.NotifyAll();
  return Reactor::local()->Terminate();
}

}  // namespace internal
}  // namespace puddle
