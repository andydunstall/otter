#pragma once

#include <string>

#include "absl/status/status.h"
#include "otter/server/protocol.h"
#include "puddle/writer.h"

namespace otter {
namespace server {

// Writer manages writing the underlying writer.
//
// Writes will first write to the writer buffer, then can be flushed with
// Flush. If the buffer becomes full, writes will flush the buffer then
// continue writing to the buffer.
class Writer {
 public:
  Writer(puddle::Writer* writer, size_t buf_size);

  absl::Status WriteUint16(uint16_t n);

  absl::Status WriteUint32(uint32_t n);

  absl::Status WriteMessageType(MessageType message_type);

  absl::Status WriteString(const std::string_view& s);

  absl::Status WriteHeader(MessageType message_type);

  absl::Status Flush();

 private:
  puddle::Writer* writer_;

  std::vector<uint8_t> buf_;

  // The offset of the start of the 'free' buffer in buf_.
  size_t offset_ = 0;
};

}  // namespace server
}  // namespace otter
