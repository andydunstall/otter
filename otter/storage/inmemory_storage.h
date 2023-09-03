#pragma once

#include "otter/storage/storage.h"

namespace otter {
namespace storage {

class InMemoryStorage : public Storage {
 public:
  absl::StatusOr<std::string> Get(const std::string_view& key) override;

  absl::Status Put(const std::string_view& key,
                   const std::string_view& value) override;

  absl::Status Delete(const std::string_view& key) override;

 private:
  std::unordered_map<std::string, std::string> store_;
};

}  // namespace storage
}  // namespace otter
