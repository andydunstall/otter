#include "puddle/pprof/pprof.h"

#include "gperftools/profiler.h"

namespace puddle {
namespace pprof {

void Start(Config config) {
  if (config.path == "") {
    return;
  }

  ProfilerStart(config.path.c_str());
}

void Stop() { ProfilerStop(); }

}  // namespace pprof
}  // namespace puddle
