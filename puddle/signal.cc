#include "puddle/signal.h"

#include <csignal>

namespace puddle {

// Store the handler globally as sigaction needs a function pointer.
std::function<void(int)> handler;

void Dispatcher(int signal) {
  if (handler) {
    handler(signal);
  }
}

void NotifySignal(const std::vector<int>& signals,
                  std::function<void(int)> fn) {
  handler = std::move(fn);

  struct sigaction sa;
  sa.sa_handler = Dispatcher;
  sa.sa_flags = 0;

  for (int signal : signals) {
    sigaction(signal, &sa, nullptr);
  }
}

}  // namespace puddle
