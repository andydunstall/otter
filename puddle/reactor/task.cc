#include "puddle/reactor/task.h"

#include "puddle/reactor/reactor.h"

namespace puddle {
namespace reactor {

void Task::Join() { context_->Join(); }

}  // namespace reactor
}  // namespace puddle
