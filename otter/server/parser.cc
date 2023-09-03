#include "otter/server/parser.h"

#include <arpa/inet.h>

#include <cstring>

#include "absl/strings/str_format.h"

namespace otter {
namespace server {

Parser::Parser(const absl::Span<uint8_t> buf) : buf_{buf}, offset_{0} {}

std::optional<uint16_t> Parser::ReadUint16() {
  if (buf_.size() < offset_ + sizeof(uint16_t)) {
    return std::nullopt;
  }

  uint16_t n_net;
  memcpy(&n_net, buf_.data() + offset_, sizeof(uint16_t));
  offset_ += sizeof(uint16_t);
  return htons(n_net);
}

std::optional<uint32_t> Parser::ReadUint32() {
  if (buf_.size() < offset_ + sizeof(uint32_t)) {
    return std::nullopt;
  }

  uint32_t n_net;
  memcpy(&n_net, buf_.data() + offset_, sizeof(uint32_t));
  offset_ += sizeof(uint32_t);
  return htonl(n_net);
}

absl::StatusOr<std::optional<std::string_view>> Parser::ReadString() {
  std::optional<uint32_t> string_size = ReadUint32();
  if (!string_size) {
    return std::nullopt;
  }
  if (buf_.size() < offset_ + *string_size) {
    return absl::InvalidArgumentError(
        absl::StrFormat("string size exceeds payload size: %d", *string_size));
  }
  std::string_view s{(char*)buf_.data() + offset_, *string_size};
  offset_ += *string_size;
  return s;
}

std::optional<MessageType> Parser::ReadMessageType() {
  std::optional<uint16_t> n = ReadUint16();
  if (!n) {
    return std::nullopt;
  }
  return static_cast<MessageType>(*n);
}

std::optional<Header> Parser::ReadHeader() {
  std::optional<MessageType> message_type = ReadMessageType();
  if (!message_type) {
    return std::nullopt;
  }

  std::optional<uint16_t> protocol_version = ReadUint16();
  if (!protocol_version) {
    return std::nullopt;
  }

  std::optional<uint32_t> payload_size = ReadUint32();
  if (!payload_size) {
    return std::nullopt;
  }

  Header header;
  header.message_type = *message_type;
  header.protocol_version = *protocol_version;
  header.payload_size = *payload_size;
  return header;
}

}  // namespace server
}  // namespace otter
