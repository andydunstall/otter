#pragma once

#include <string>

namespace puddle {
namespace pprof {

struct Config {
  // Path to the write a pprof profile to.
  //
  // If the path is empty, pprof is disabled.
  std::string path;
};

void Start(Config config);

void Stop();

}  // namespace pprof
}  // namespace puddle
