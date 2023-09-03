#pragma once

#include <optional>

#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "otter/server/protocol.h"

namespace otter {
namespace server {

class Parser {
 public:
  Parser(const absl::Span<uint8_t> buf);

  size_t offset() const { return offset_; }

  std::optional<uint16_t> ReadUint16();

  std::optional<uint32_t> ReadUint32();

  absl::StatusOr<std::optional<std::string_view>> ReadString();

  std::optional<MessageType> ReadMessageType();

  std::optional<Header> ReadHeader();

 private:
  absl::Span<uint8_t> buf_;

  size_t offset_;
};

}  // namespace server
}  // namespace otter
