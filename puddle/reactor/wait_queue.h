#pragma once

#include <list>

namespace puddle {
namespace reactor {
namespace internal {

class Context;

class WaitQueue {
 public:
  void NotifyOne();

  void NotifyAll();

  void SuspendAndWait(Context* c);

 private:
  std::list<Context*> queue_;
};

}  // namespace internal
}  // namespace reactor
}  // namespace puddle
