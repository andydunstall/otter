#include "puddle/net/buffer.h"

#include <cstring>

namespace puddle {
namespace net {

Buffer::Buffer(size_t capacity) : buf_(capacity) {}

void Buffer::Commit(size_t n) { end_idx_ += n; }

void Buffer::Consume(size_t n) {
  if (start_idx_ + n >= buf_.size()) {
    // If we've reached the end of the buffer, we can discard all bytes.
    start_idx_ = 0;
    end_idx_ = 0;
  } else {
    start_idx_ += n;

    // If the over half the bytes in the buffer are unused (not committed and
    // not writable), shift the committed bytes to fill the unused space.
    if (2 * start_idx_ > end_idx_ && end_idx_ - start_idx_ < 512) {
      memcpy(buf_.data(), buf_.data() + start_idx_, end_idx_ - start_idx_);
      end_idx_ -= start_idx_;
      start_idx_ = 0;
    }
  }
}

void Buffer::Reserve(size_t capacity) { buf_.resize(capacity); }

}  // namespace net
}  // namespace puddle
