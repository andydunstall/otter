#include "puddle/reactor/context.h"

#include "puddle/reactor/reactor.h"

namespace puddle {
namespace reactor {
namespace internal {

Context::Context(const std::string& name) : name_{name}, ref_count_{1} {}

void Context::Join() {
  if (terminated_) return;

  join_wait_ = local()->active();
  local()->Suspend();
}

boost::context::fiber Context::Terminate() {
  terminated_ = true;
  local()->Terminate();
}

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
