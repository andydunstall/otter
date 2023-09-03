#include "fuddle/server/reply_builder.h"

#include <arpa/inet.h>

#include <cstring>

namespace fuddle {
namespace server {

ReplyBuilder::ReplyBuilder(puddle::Socket* socket) : socket_{socket} {}

void ReplyBuilder::WriteUint16(uint16_t n) {
  size_t offset = buf_.size();
  buf_.resize(offset + sizeof(uint16_t));

  uint16_t n_net = htons(n);
  memcpy(buf_.data() + offset, &n_net, sizeof(uint16_t));
}

void ReplyBuilder::WriteUint32(uint32_t n) {
  size_t offset = buf_.size();
  buf_.resize(offset + sizeof(uint32_t));

  uint32_t n_net = htonl(n);
  memcpy(buf_.data() + offset, &n_net, sizeof(uint32_t));
}

void ReplyBuilder::WriteMessageType(MessageType message_type) {
  return WriteUint16(static_cast<uint16_t>(message_type));
}

void ReplyBuilder::WriteString(const std::string& s) {
  WriteUint32(s.size());

  size_t offset = buf_.size();
  buf_.resize(offset + s.size());
  memcpy(buf_.data() + offset, s.data(), s.size());
}

void ReplyBuilder::WriteRawBytes(const absl::Span<uint8_t>& b) {
  size_t offset = buf_.size();
  buf_.resize(offset + b.size());
  memcpy(buf_.data() + offset, b.data(), b.size());
}

void ReplyBuilder::WriteHeader(Header header) {
  WriteMessageType(header.message_type);
  WriteUint16(header.protocol_version);
  WriteUint32(header.payload_size);
}

absl::Status ReplyBuilder::Flush() {
  size_t offset = 0;

  while (offset < buf_.size()) {
    absl::StatusOr<size_t> status = socket_->Write(
        absl::Span<uint8_t>(buf_.data() + offset, buf_.size() - offset));
    if (!status.ok()) {
      return status.status();
    }
    offset += *status;
  }
  return absl::OkStatus();
}

}  // namespace server
}  // namespace fuddle
