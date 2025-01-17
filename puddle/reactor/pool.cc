#include "puddle/reactor/pool.h"

#include <thread>

#include "puddle/reactor/reactor.h"
#include "puddle/shard/shard.h"

namespace puddle {
namespace reactor {

Pool::Pool(Config config) : config_{config} {}

void Pool::OnAllShards(std::function<void()> f) {
  int n_threads = config_.threads;

  std::vector<std::thread> threads;
  for (int shard_id = 0; shard_id != n_threads; shard_id++) {
    threads.emplace_back(
        [&](int shard_id) {
          reactor::Reactor::Setup(config_);
          shard::set_local({.id = shard_id});
          f();
        },
        shard_id);

    int cpu_id = shard_id;
    if (!config_.cpu_set.empty()) {
      cpu_id = config_.cpu_set[shard_id];
    }

    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(cpu_id, &cpu_set);
    pthread_setaffinity_np(threads[shard_id].native_handle(), sizeof(cpu_set_t),
                           &cpu_set);
  }
  for (auto& thread : threads) {
    thread.join();
  }
}

}  // namespace reactor
}  // namespace puddle
