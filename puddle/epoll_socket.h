#pragma once

#include "puddle/socket.h"

namespace puddle {

class EpollSocket : public Socket {
 public:
  EpollSocket(int fd = -1);

  ~EpollSocket();

  EpollSocket(const EpollSocket&) = delete;
  EpollSocket& operator=(const EpollSocket&) = delete;

  EpollSocket(EpollSocket&&);
  EpollSocket& operator=(EpollSocket&&);

  std::unique_ptr<Socket> Accept() override;

  absl::StatusOr<size_t> Read(Buffer* buf) override;

  absl::StatusOr<size_t> Write(const absl::Span<uint8_t>& buf) override;

  static EpollSocket Open();
};

}  // namespace puddle
