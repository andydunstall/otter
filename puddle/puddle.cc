#include "puddle/puddle.h"

#include "puddle/internal/reactor.h"

namespace puddle {

void Start() { internal::Reactor::Start(); }

}  // namespace puddle
