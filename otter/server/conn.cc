#include "otter/server/conn.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"

namespace otter {
namespace server {

Conn::Conn(std::unique_ptr<puddle::Socket> socket,
           std::shared_ptr<storage::Storage> storage)
    : socket_{std::move(socket)},
      reader_{socket_.get(), 2048, 16384},
      writer_{socket_.get(), 2048},
      storage_{storage} {}

void Conn::ReadLoop() {
  while (true) {
    absl::StatusOr<MessageType> message_type = reader_.ReadHeader();
    if (!message_type.ok()) {
      DLOG(INFO) << "conn: read loop: read error; err="
                 << message_type.status();
      return;
    }

    DLOG(INFO) << "conn: read loop: read header; message_type="
               << static_cast<uint16_t>(*message_type);

    absl::Status status;
    switch (*message_type) {
      case MessageType::kPing:
        status = Ping();
        break;
      case MessageType::kGet:
        status = Get();
        break;
      case MessageType::kPut:
        status = Put();
        break;
      case MessageType::kDelete:
        status = Delete();
        break;
      default:
        status = absl::InvalidArgumentError(
            absl::StrFormat("unsupported request type: %d",
                            static_cast<uint16_t>(*message_type)));
        break;
    }
    if (!status.ok()) {
      LOG(WARNING) << "conn: read loop: " << status;
      return;
    }
  }
}

absl::Status Conn::Ping() {
  DLOG(INFO) << "ping";

  absl::Status status;
  status = writer_.WriteHeader(MessageType::kPong);
  if (!status.ok()) {
    return status;
  }
  return writer_.Flush();
}

absl::Status Conn::Get() {
  absl::StatusOr<std::string> key = reader_.ReadString();
  if (!key.ok()) {
    return key.status();
  }

  absl::StatusOr<std::string> value = storage_->Get(*key);

  absl::Status status;
  status = writer_.WriteHeader(MessageType::kData);
  if (!status.ok()) {
    return status;
  }

  if (value.ok()) {
    DLOG(INFO) << "conn: get: key=" << *key << "; value=" << *value;

    // Status code.
    status = writer_.WriteUint16(static_cast<uint16_t>(absl::StatusCode::kOk));
    if (!status.ok()) {
      return status;
    }
    // Value.
    status = writer_.WriteString(*value);
    if (!status.ok()) {
      return status;
    }
  } else {
    DLOG(INFO) << "conn: get: key=" << *key << "; status=" << value.status();

    // Status code.
    status = writer_.WriteUint16(static_cast<uint16_t>(value.status().code()));
    if (!status.ok()) {
      return status;
    }
  }

  return writer_.Flush();
}

absl::Status Conn::Put() {
  absl::StatusOr<std::string> key = reader_.ReadString();
  if (!key.ok()) {
    return key.status();
  }

  absl::StatusOr<std::string> value = reader_.ReadString();
  if (!value.ok()) {
    return value.status();
  }

  DLOG(INFO) << "conn: put: key=" << *key << "; value=" << *value;

  absl::Status put_status = storage_->Put(*key, *value);

  absl::Status status;
  status = writer_.WriteHeader(MessageType::kAck);
  if (!status.ok()) {
    return status;
  }
  status = writer_.WriteUint16(static_cast<uint16_t>(put_status.code()));
  if (!status.ok()) {
    return status;
  }
  return writer_.Flush();
}

absl::Status Conn::Delete() {
  absl::StatusOr<std::string> key = reader_.ReadString();
  if (!key.ok()) {
    return key.status();
  }

  DLOG(INFO) << "conn: delete: key=" << *key;

  absl::Status delete_status = storage_->Delete(*key);

  absl::Status status;
  status = writer_.WriteHeader(MessageType::kAck);
  if (!status.ok()) {
    return status;
  }
  status = writer_.WriteUint16(static_cast<uint16_t>(delete_status.code()));
  if (!status.ok()) {
    return status;
  }
  return writer_.Flush();
}

}  // namespace server
}  // namespace otter
