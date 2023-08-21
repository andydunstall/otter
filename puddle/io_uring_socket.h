#pragma once

#include "puddle/socket.h"

namespace puddle {

class IoUringShard;

class IoUringSocket : public Socket {
 public:
  IoUringSocket(int fd = -1, IoUringShard* shard = nullptr);

  ~IoUringSocket();

  IoUringSocket(const IoUringSocket&) = delete;
  IoUringSocket& operator=(const IoUringSocket&) = delete;

  IoUringSocket(IoUringSocket&&);
  IoUringSocket& operator=(IoUringSocket&&);

  std::unique_ptr<Socket> Accept() override;

  absl::StatusOr<size_t> Read(Buffer* buf) override;

  absl::StatusOr<size_t> Write(const absl::Span<uint8_t>& buf) override;

  static IoUringSocket Open(IoUringShard* shard);

 private:
  IoUringShard* shard_;
};

}  // namespace puddle
