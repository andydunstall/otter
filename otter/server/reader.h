#pragma once

#include <vector>

#include "absl/status/statusor.h"
#include "otter/server/protocol.h"
#include "puddle/reader.h"

namespace otter {
namespace server {

// Reader handles reading messages from the given network reader.
//
// If the initial buffer is filled it will be extended up to the maximum buffer
// size. If a message cannot be read without exceeding the maximum buf size
// then an error is returned.
class Reader {
 public:
  Reader(puddle::Reader* reader, size_t init_buf_size, size_t max_buf_size);

  absl::StatusOr<uint16_t> ReadUint16();

  absl::StatusOr<uint32_t> ReadUint32();

  absl::StatusOr<MessageType> ReadMessageType();

  absl::StatusOr<MessageType> ReadHeader();

  absl::StatusOr<std::string> ReadString();

 private:
  size_t pending_size() const { return end_idx_ - start_idx_; }

  absl::Status ReadAtLeast(size_t n);

  puddle::Reader* reader_;

  // Points to the index of the start of the pending data in the buffer.
  size_t start_idx_ = 0;

  // Points to the index of the end of the pending data in the buffer.
  size_t end_idx_ = 0;

  std::vector<uint8_t> buf_;
};

}  // namespace server
}  // namespace otter
