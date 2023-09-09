#pragma once

#include "absl/status/statusor.h"
#include "absl/types/span.h"

namespace puddle {

class Reader {
 public:
  virtual ~Reader() = default;

  virtual absl::StatusOr<size_t> Read(absl::Span<uint8_t> buf) = 0;
};

}  // namespace puddle
