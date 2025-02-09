#pragma once

#include <functional>
#include <vector>

namespace puddle {

// TODO(andydunstall): Move into runtime to shutdown gracefully.
void NotifySignal(const std::vector<int>& signals, std::function<void(int)> fn);

}  // namespace puddle
