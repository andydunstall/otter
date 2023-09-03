#include "otter/storage/inmemory_storage.h"

#include "absl/log/check.h"

namespace otter {
namespace storage {

absl::StatusOr<std::string> InMemoryStorage::Get(const std::string_view& key) {
  if (auto item = store_.find(std::string(key)); item != store_.end()) {
    return item->second;
  } else {
    return absl::NotFoundError("not found");
  }
}

absl::Status InMemoryStorage::Put(const std::string_view& key,
                                  const std::string_view& value) {
  store_.emplace(std::string(key), std::string(value));
  return absl::OkStatus();
}

absl::Status InMemoryStorage::Delete(const std::string_view& key) {
  store_.erase(std::string(key));
  return absl::OkStatus();
}

}  // namespace storage
}  // namespace otter
