#pragma once

namespace otter {
namespace server {

constexpr size_t kMaxPayloadSize = 64 * (1 << 20);  // 64 MiB
constexpr size_t kMaxKeySize = 1 << 20;             // 1 MiB
constexpr size_t kMaxValueSize = 32 * (1 << 20);    // 32 MiB

enum class MessageType : uint16_t {
  kEcho = 1,
  kGet = 2,
  kPut = 3,
  kDelete = 4,
  kAck = 5,
  kData = 6,
};

struct Header {
  MessageType message_type;
  uint16_t protocol_version;
  uint32_t payload_size;
};

}  // namespace server
}  // namespace otter
