#pragma once

#include "otter/storage/storage.h"
#include "rocksdb/db.h"

namespace otter {
namespace storage {

class RocksDBStorage : public Storage {
 public:
  RocksDBStorage(rocksdb::DB* db = nullptr);

  ~RocksDBStorage();

  RocksDBStorage(const RocksDBStorage&) = delete;
  RocksDBStorage& operator=(const RocksDBStorage&) = delete;

  RocksDBStorage(RocksDBStorage&&);
  RocksDBStorage& operator=(RocksDBStorage&&);

  absl::StatusOr<std::string> Get(const std::string_view& key) override;

  absl::Status Put(const std::string_view& key,
                   const std::string_view& value) override;

  absl::Status Delete(const std::string_view& key) override;

  static absl::StatusOr<RocksDBStorage> Open(const std::string& path);

 private:
  rocksdb::DB* db_;
};

}  // namespace storage
}  // namespace otter
