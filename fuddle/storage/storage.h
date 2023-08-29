#pragma once

#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace fuddle {
namespace storage {

class Storage {
 public:
  ~Storage() = default;

  virtual absl::StatusOr<std::string> Get(const std::string_view& key) = 0;

  virtual absl::Status Put(const std::string_view& key,
                           const std::string_view& value) = 0;

  virtual absl::Status Delete(const std::string_view& key) = 0;
};

}  // namespace storage
}  // namespace fuddle
