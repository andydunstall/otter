#include "puddle/task.h"

namespace puddle {

void Task::Join() { context_->Join(); }

void Task::Detach() {
  // TODO(andydunstall)
}

}  // namespace puddle
