#pragma once

#include "fuddle/server/conn.h"
#include "fuddle/storage/storage.h"
#include "puddle/listener.h"

namespace fuddle {
namespace server {

class Listener : public puddle::Listener {
 public:
  Listener(std::shared_ptr<storage::Storage> storage);

  void Connection(std::unique_ptr<puddle::Socket> socket) override;

 private:
  std::shared_ptr<storage::Storage> storage_;
};

}  // namespace server
}  // namespace fuddle
