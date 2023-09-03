#pragma once

#include "otter/server/conn.h"
#include "otter/storage/storage.h"
#include "puddle/listener.h"

namespace otter {
namespace server {

class Listener : public puddle::Listener {
 public:
  Listener(std::shared_ptr<storage::Storage> storage);

  void Connection(std::unique_ptr<puddle::Socket> socket) override;

 private:
  std::shared_ptr<storage::Storage> storage_;
};

}  // namespace server
}  // namespace otter
