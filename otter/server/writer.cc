#include "otter/server/writer.h"

#include <arpa/inet.h>

#include <cstring>

#include "absl/log/log.h"

namespace otter {
namespace server {

Writer::Writer(puddle::Writer* writer, size_t buf_size)
    : writer_{writer}, buf_(buf_size) {}

absl::Status Writer::WriteUint16(uint16_t n) {
  if (buf_.size() < offset_ + sizeof(uint16_t)) {
    // Insufficient bytes so flush first.
    absl::Status status = Flush();
    if (!status.ok()) {
      return status;
    }
  }

  uint16_t n_net = htons(n);
  memcpy(buf_.data() + offset_, &n_net, sizeof(uint16_t));
  offset_ += sizeof(uint16_t);
  return absl::OkStatus();
}

absl::Status Writer::WriteUint32(uint32_t n) {
  if (buf_.size() < offset_ + sizeof(uint32_t)) {
    // Insufficient bytes so flush first.
    absl::Status status = Flush();
    if (!status.ok()) {
      return status;
    }
  }

  uint32_t n_net = htonl(n);
  memcpy(buf_.data() + offset_, &n_net, sizeof(uint32_t));
  offset_ += sizeof(uint32_t);
  return absl::OkStatus();
}

absl::Status Writer::WriteMessageType(MessageType message_type) {
  return WriteUint16(static_cast<uint16_t>(message_type));
}

absl::Status Writer::WriteString(const std::string_view& s) {
  absl::Status status = WriteUint32(s.size());
  if (!status.ok()) {
    return status;
  }

  // TODO(andydunstall): If s is large, instead flush the buffer then write
  // s directly to the socket to avoid the extra copy. Could use writev to
  // flush both in a single system call.

  size_t written = 0;
  while (written < s.size()) {
    size_t remaining = buf_.size() - offset_;

    // Write as many bytes to the buffer as we can.
    size_t n = s.size() - written;
    if (n > remaining) {
      n = remaining;
    }
    memcpy(buf_.data() + offset_, s.data() + written, n);
    written += n;
    offset_ += n;

    // If we've filled buf_, flush it and keep writing to buf_.
    if (offset_ == buf_.size()) {
      status = Flush();
      if (!status.ok()) {
        return status;
      }
    }
  }
  return absl::OkStatus();
}

absl::Status Writer::WriteHeader(MessageType message_type) {
  absl::Status status = WriteMessageType(message_type);
  if (!status.ok()) {
    return status;
  }
  status = WriteUint16(1);
  if (!status.ok()) {
    return status;
  }
  return absl::OkStatus();
}

absl::Status Writer::Flush() {
  DLOG(INFO) << "writer: flush; offset=" << offset_;

  // Keep writing until all bytes in the buffer have been written then
  // reset offset_.
  size_t written = 0;
  while (written < offset_) {
    absl::StatusOr<size_t> status = writer_->Write(
        absl::Span<uint8_t>(buf_.data() + written, offset_ - written));
    if (!status.ok()) {
      return status.status();
    }
    written += *status;
  }
  offset_ = 0;
  return absl::OkStatus();
}

}  // namespace server
}  // namespace otter
