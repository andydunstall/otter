#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "absl/types/span.h"

namespace puddle {
namespace net {

// Buffer contains a buffer to write read bytes to.
//
// It is split into consumed, committed and free bytes.
class Buffer {
 public:
  Buffer(size_t capacity = 256);

  size_t committed() const { return end_idx_ - start_idx_; }

  size_t capacity() const { return buf_.size(); }

  bool full() const { return end_idx_ == buf_.size(); }

  // Returns the committed buffer.
  absl::Span<uint8_t> committed_buf() {
    return absl::Span<uint8_t>{buf_.data() + start_idx_, end_idx_ - start_idx_};
  }

  // Returns the uncommitted buffer to write to.
  absl::Span<uint8_t> write_buf() {
    return absl::Span<uint8_t>{buf_.data() + end_idx_, buf_.size() - end_idx_};
  }

  void Commit(size_t n);

  // Marks uncommits n bytes freeing then for further writes.
  void Consume(size_t n);

  void Reserve(size_t capacity);

 private:
  // Points to the index of the start of the consumed data in the buffer.
  size_t start_idx_ = 0;

  // Points to the index of the end of the consumed data in the buffer.
  size_t end_idx_ = 0;

  // Stores the underlying buffer.
  std::vector<uint8_t> buf_;
};

}  // namespace net
}  // namespace puddle
