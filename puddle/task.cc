#include "puddle/task.h"

namespace puddle {

void Task::Join() {
  assert(context_ != nullptr);
  context_->Join();
}

void Task::Detach() {
  assert(context_ != nullptr);
  context_.reset();
}

Task::Task(boost::intrusive_ptr<internal::Context> context)
    : context_(context) {}

}  // namespace puddle
