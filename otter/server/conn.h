#pragma once

#include <memory>

#include "otter/storage/storage.h"
#include "puddle/socket.h"

namespace otter {
namespace server {

class Conn {
 public:
  Conn(std::unique_ptr<puddle::Socket> socket,
       std::shared_ptr<storage::Storage> storage);

  void ReadLoop();

 private:
  absl::Status Echo(absl::Span<uint8_t> b);

  absl::Status Get(absl::Span<uint8_t> b);

  absl::Status Put(absl::Span<uint8_t> b);

  absl::Status Delete(absl::Span<uint8_t> b);

  std::unique_ptr<puddle::Socket> socket_;

  std::shared_ptr<storage::Storage> storage_;
};

}  // namespace server
}  // namespace otter
