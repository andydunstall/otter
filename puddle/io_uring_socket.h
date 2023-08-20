#pragma once

#include <liburing.h>

#include "puddle/socket.h"

namespace puddle {

class IoUringSocket : public Socket {
 public:
  IoUringSocket(int fd = -1, struct io_uring* ring = nullptr);

  ~IoUringSocket();

  IoUringSocket(const IoUringSocket&) = delete;
  IoUringSocket& operator=(const IoUringSocket&) = delete;

  IoUringSocket(IoUringSocket&&);
  IoUringSocket& operator=(IoUringSocket&&);

  std::unique_ptr<Socket> Accept() override;

  absl::StatusOr<size_t> Read(Buffer* buf) override;

  absl::StatusOr<size_t> Write(const absl::Span<uint8_t>& buf) override;

  static IoUringSocket Open(struct io_uring* ring);

 private:
  struct io_uring* ring_;
};

}  // namespace puddle
