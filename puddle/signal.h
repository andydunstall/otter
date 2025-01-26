#pragma once

#include <vector>

namespace puddle {

template <typename Fn>
void NotifySignal(const std::vector<int>& signals, Fn&& fn) {}

}  // namespace puddle
