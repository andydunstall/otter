#include "puddle/shard.h"

namespace puddle {

std::unique_ptr<Socket> Shard::OpenSocket() {
  return std::make_unique<Socket>(Socket::Open());
}

}  // namespace puddle
