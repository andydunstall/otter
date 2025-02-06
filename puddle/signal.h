#pragma once

#include <functional>
#include <vector>

namespace puddle {

void NotifySignal(const std::vector<int>& signals, std::function<void(int)> fn);

}  // namespace puddle
