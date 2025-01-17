#include "puddle/shard/shard.h"

namespace puddle {
namespace shard {

thread_local Shard tl_shard = {.id = -1};

Shard local() { return tl_shard; }

int id() { return tl_shard.id; }

void set_local(Shard shard) { tl_shard = shard; }

}  // namespace shard
}  // namespace puddle
