#include "puddle/internal/context.h"

namespace puddle {
namespace internal {

boost::context::fiber Context::Terminate() {
  // TODO(andydunstall)
  return {};
}

}  // namespace internal
}  // namespace puddle
