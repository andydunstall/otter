#pragma once

#include "absl/status/statusor.h"
#include "absl/types/span.h"

namespace puddle {

class Writer {
 public:
  virtual ~Writer() = default;

  virtual absl::StatusOr<size_t> Write(const absl::Span<uint8_t>& buf) = 0;
};

}  // namespace puddle
