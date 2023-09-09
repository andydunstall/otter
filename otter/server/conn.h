#pragma once

#include <memory>

#include "otter/server/reader.h"
#include "otter/server/writer.h"
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
  absl::Status Ping();

  absl::Status Get();

  absl::Status Put();

  absl::Status Delete();

  std::unique_ptr<puddle::Socket> socket_;

  Reader reader_;

  Writer writer_;

  std::shared_ptr<storage::Storage> storage_;
};

}  // namespace server
}  // namespace otter
