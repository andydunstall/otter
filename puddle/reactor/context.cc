#include "puddle/reactor/context.h"

#include "puddle/reactor/reactor.h"

namespace puddle {
namespace reactor {
namespace internal {

Context::Context(const std::string& name) : name_{name}, ref_count_{1} {}

Context::~Context() { assert(!ready_hook_.is_linked()); }

void Context::Join() {
  if (terminated_) return;

  wait_queue_.SuspendAndWait(local()->active());
}

void Context::Suspend() { local()->Suspend(); }

void Context::Wake() { local()->scheduler()->AddReady(this); }

boost::context::fiber Context::Terminate() {
  terminated_ = true;
  wait_queue_.NotifyAll();
  return local()->Terminate();
}

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
