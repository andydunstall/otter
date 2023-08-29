#pragma once

#include "fuddle/storage/storage.h"
#include "rocksdb/db.h"

namespace fuddle {
namespace storage {

class RocksDBStorage : public Storage {
 public:
  RocksDBStorage(rocksdb::DB* db = nullptr);

  ~RocksDBStorage();

  absl::StatusOr<std::string> Get(const std::string_view& key) override;

  absl::Status Put(const std::string_view& key,
                   const std::string_view& value) override;

  absl::Status Delete(const std::string_view& key) override;

  static absl::StatusOr<RocksDBStorage> Open(const std::string& path);

 private:
  rocksdb::DB* db_;
};

}  // namespace storage
}  // namespace fuddle
