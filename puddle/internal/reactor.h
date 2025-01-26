#pragma once

namespace puddle {
namespace internal {

// Reactor manages scheduling tasks and asynchronous IO.
class Reactor {
 public:
  static void Start();
};

}  // namespace internal
}  // namespace puddle
