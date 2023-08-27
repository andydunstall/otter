#pragma once

#include "absl/types/span.h"

namespace fuddle {
namespace server {

enum class MessageType : uint16_t {
  kPing = 1,
  kPong = 2,
};

size_t ReadUint16(absl::Span<uint8_t> b, size_t offset, uint16_t* n);

size_t ReadUint64(absl::Span<uint8_t> b, size_t offset, uint64_t* n);

size_t ReadMessageType(absl::Span<uint8_t> b, size_t offset, MessageType* type);

}  // namespace server
}  // namespace fuddle
