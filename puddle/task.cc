#include "puddle/task.h"

namespace puddle {

void Task::Join() { context_->Join(); }

void Task::Detach() {
  // TODO(andydunstall)
}

Task::Task(boost::intrusive_ptr<internal::Context> context)
    : context_(context) {}

}  // namespace puddle
