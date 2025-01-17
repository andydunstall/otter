#pragma once

#include <cstddef>

namespace puddle {
namespace shard {

// Shard contains state on the local shard.
//
// Note this could have been in core::reactor, though it is used by logger
// so to avoid a circular dependency it has it's own package.
struct Shard {
  // The shards ID.
  int id;
};

// local returns the threads local shard state.
//
// May return nullptr if the thread doesn't have a shard.
Shard local();

// Returns the shard ID of the local shard (or -1 if there is no local shard).
int id();

void set_local(Shard shard);

}  // namespace shard
}  // namespace puddle
