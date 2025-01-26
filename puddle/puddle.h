#pragma once

#include <chrono>

#include "puddle/task.h"

namespace puddle {

void Start();

// Spawn a task (user-space thread).
template <typename Fn, typename... Arg>
Task Spawn(Fn&& fn, Arg&&... arg) {
  return Task{};
}

template <typename Rep, typename Period>
void SleepFor(const std::chrono::duration<Rep, Period>& duration) {}

}  // namespace puddle
