#include "server/protocol.h"

#include <arpa/inet.h>
#include <endian.h>

#include <cstring>

namespace fuddle {
namespace server {

size_t ReadUint16(absl::Span<uint8_t> b, size_t offset, uint16_t* n) {
  if (b.size() < offset + sizeof(uint16_t)) {
    return 0;
  }

  uint16_t n_net;
  memcpy(&n_net, b.data() + offset, sizeof(uint16_t));
  *n = ntohs(n_net);
  return offset + sizeof(uint16_t);
}

size_t ReadUint64(absl::Span<uint8_t> b, size_t offset, uint64_t* n) {
  if (b.size() < offset + sizeof(uint64_t)) {
    return 0;
  }

  uint64_t n_net;
  memcpy(&n_net, b.data() + offset, sizeof(uint64_t));
  *n = be64toh(n_net);
  return offset + sizeof(uint64_t);
}

size_t ReadMessageType(absl::Span<uint8_t> b, size_t offset,
                       MessageType* type) {
  uint16_t type_n;
  offset = ReadUint16(b, offset, &type_n);
  if (offset == 0) {
    return 0;
  }
  *type = static_cast<MessageType>(type_n);
  return offset;
}

}  // namespace server
}  // namespace fuddle
