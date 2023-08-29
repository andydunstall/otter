#include "fuddle/storage/rocksdb_storage.h"

#include "absl/log/check.h"

namespace fuddle {
namespace storage {

RocksDBStorage::RocksDBStorage(rocksdb::DB* db) : db_{db} {}

RocksDBStorage::~RocksDBStorage() {
  if (db_) {
    delete db_;
  }
}

absl::StatusOr<std::string> RocksDBStorage::Get(const std::string_view& key) {
  std::string value;
  rocksdb::Status s = db_->Get(rocksdb::ReadOptions(), key, &value);
  if (!s.ok()) {
    return absl::UnknownError(s.ToString());
  }
  return value;
}

absl::Status RocksDBStorage::Put(const std::string_view& key,
                                 const std::string_view& value) {
  rocksdb::Status s = db_->Put(rocksdb::WriteOptions(), key, value);
  if (!s.ok()) {
    return absl::UnknownError(s.ToString());
  }
  return absl::OkStatus();
}

absl::Status RocksDBStorage::Delete(const std::string_view& key) {
  rocksdb::Status s = db_->Delete(rocksdb::WriteOptions(), key);
  if (!s.ok()) {
    return absl::UnknownError(s.ToString());
  }
  return absl::OkStatus();
}

absl::StatusOr<RocksDBStorage> Open(const std::string& path) {
  rocksdb::DB* db;
  rocksdb::Options options;
  options.create_if_missing = true;
  rocksdb::Status s = rocksdb::DB::Open(options, path, &db);
  if (!s.ok()) {
    return absl::UnknownError(s.ToString());
  }
  return RocksDBStorage{db};
}

}  // namespace storage
}  // namespace fuddle
