#include "puddle/puddle.h"

#include "puddle/runtime.h"

namespace puddle {

thread_local Runtime* local;

void Start() { local = new Runtime{}; }

}  // namespace puddle
