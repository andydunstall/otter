#pragma once

namespace otter {
namespace server {

constexpr size_t kProtocolVersion = 1;

enum class MessageType : uint16_t {
  kPing = 1,
  kPong = 2,
  kGet = 3,
  kPut = 4,
  kDelete = 5,
  kAck = 6,
  kData = 7,
};

}  // namespace server
}  // namespace otter
