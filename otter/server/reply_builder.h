#pragma once

#include <cstdint>
#include <vector>

#include "absl/status/status.h"
#include "otter/server/protocol.h"
#include "puddle/socket.h"

namespace otter {
namespace server {

class ReplyBuilder {
 public:
  ReplyBuilder(puddle::Socket* socket);

  void WriteUint16(uint16_t n);

  void WriteUint32(uint32_t n);

  void WriteMessageType(MessageType message_type);

  void WriteString(const std::string& s);

  void WriteRawBytes(const absl::Span<uint8_t>& b);

  void WriteHeader(Header header);

  absl::Status Flush();

 private:
  std::vector<uint8_t> buf_;

  puddle::Socket* socket_;
};

}  // namespace server
}  // namespace otter
