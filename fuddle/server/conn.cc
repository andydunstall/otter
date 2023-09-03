#include "fuddle/server/conn.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"
#include "fuddle/server/parser.h"
#include "fuddle/server/protocol.h"
#include "fuddle/server/reply_builder.h"

namespace fuddle {
namespace server {

Conn::Conn(std::unique_ptr<puddle::Socket> socket,
           std::shared_ptr<storage::Storage> storage)
    : socket_{std::move(socket)}, storage_{storage} {}

void Conn::ReadLoop() {
  puddle::Buffer recv_buf;
  while (true) {
    absl::StatusOr<size_t> read_n = socket_->Read(&recv_buf);
    if (!read_n.ok()) {
      DLOG(INFO) << "conn: read loop: read error; err=" << read_n.status();
      return;
    }
    CHECK(*read_n > 0);
    recv_buf.Commit(*read_n);

    DLOG(INFO) << "conn: read loop: read bytes; bytes=" << *read_n;

    Parser parser{recv_buf.committed_buf()};

    absl::StatusOr<std::optional<Header>> header_status = parser.ReadHeader();
    if (!header_status.ok()) {
      LOG(WARNING) << "conn: read loop: failed to read header "
                   << header_status.status();
      return;
    }
    std::optional<Header> header = *header_status;
    if (!header) {
      // Insuffient bytes to read header.
      DLOG(INFO) << "conn: read loop: insufficient bytes to read header";
      // TODO(andydunstall) grow if recv buf full
      continue;
    }
    if (header->payload_size > kMaxPayloadSize) {
      LOG(WARNING) << "conn: read loop: payload size exceeds limit";
      return;
    }
    if (recv_buf.committed_buf().size() <
        parser.offset() + header->payload_size) {
      // Insuffient bytes to read payload.
      DLOG(INFO) << "conn: read loop: insufficient bytes to read payload";
      // TODO(andydunstall) grow if recv buf full
      continue;
    }

    DLOG(INFO) << "conn: read loop: read header; message_type="
               << static_cast<uint16_t>(header->message_type)
               << "; payload_size=" << header->payload_size;

    absl::Span<uint8_t> payload(
        recv_buf.committed_buf().begin() + parser.offset(),
        header->payload_size);
    recv_buf.Consume(parser.offset() + header->payload_size);

    absl::Status status;
    switch (header->message_type) {
      case MessageType::kEcho:
        status = Echo(payload);
        break;
      case MessageType::kGet:
        status = Get(payload);
        break;
      case MessageType::kPut:
        status = Put(payload);
        break;
      case MessageType::kDelete:
        status = Delete(payload);
        break;
      default:
        status = absl::InvalidArgumentError(
            absl::StrFormat("unsupported request type: %d",
                            static_cast<uint16_t>(header->message_type)));
        break;
    }
    if (!status.ok()) {
      LOG(WARNING) << "conn: read loop: " << status;
      return;
    }
  }
}

absl::Status Conn::Echo(absl::Span<uint8_t> b) {
  Header header;
  header.message_type = MessageType::kEcho;
  header.protocol_version = 1;
  header.payload_size = b.size();

  DLOG(INFO) << "conn: echo: send echo response; message_type="
             << static_cast<uint16_t>(header.message_type)
             << "; payload_size=" << header.payload_size;

  // Echo back the payload.
  ReplyBuilder reply_builder{socket_.get()};
  reply_builder.WriteHeader(header);
  reply_builder.WriteRawBytes(b);
  return reply_builder.Flush();
}

absl::Status Conn::Get(absl::Span<uint8_t> b) {
  Parser parser{b};
  absl::StatusOr<std::optional<std::string_view>> key_status =
      parser.ReadString();
  if (!key_status.ok()) {
    return key_status.status();
  }

  std::optional<std::string_view> key = *key_status;
  if (!key) {
    // We've read the whole payload so if we cannot read the requested string
    // size if means there is a protocol error.
    return absl::InvalidArgumentError("insufficient bytes to read key");
  }

  ReplyBuilder reply_builder{socket_.get()};
  Header header;
  header.message_type = MessageType::kData;
  header.protocol_version = 1;

  absl::StatusOr<std::string> value = storage_->Get(*key);

  if (value.ok()) {
    DLOG(INFO) << "conn: get: key=" << *key << "; value=" << *value;

    header.payload_size = sizeof(uint16_t) + sizeof(uint32_t) + value->size();
    reply_builder.WriteHeader(header);

    // Status code.
    reply_builder.WriteUint16(static_cast<uint16_t>(absl::StatusCode::kOk));
    // Value.
    reply_builder.WriteString(*value);
  } else {
    DLOG(INFO) << "conn: get: key=" << *key << "; status=" << value.status();

    header.payload_size = sizeof(uint16_t);
    reply_builder.WriteHeader(header);

    // Status code.
    reply_builder.WriteUint16(static_cast<uint16_t>(value.status().code()));
  }

  return reply_builder.Flush();
}

absl::Status Conn::Put(absl::Span<uint8_t> b) {
  Parser parser{b};
  absl::StatusOr<std::optional<std::string_view>> key_status =
      parser.ReadString();
  if (!key_status.ok()) {
    return key_status.status();
  }
  std::optional<std::string_view> key = *key_status;
  if (!key) {
    // We've read the whole payload so if we cannot read the requested string
    // size if means there is a protocol error.
    return absl::InvalidArgumentError("insufficient bytes to read key");
  }
  if (key->size() > kMaxKeySize) {
    return absl::InvalidArgumentError("key size exceeds limit");
  }

  absl::StatusOr<std::optional<std::string_view>> value_status =
      parser.ReadString();
  if (!value_status.ok()) {
    return value_status.status();
  }
  std::optional<std::string_view> value = *value_status;
  if (!value) {
    // We've read the whole payload so if we cannot read the requested string
    // size if means there is a protocol error.
    return absl::InvalidArgumentError("insufficient bytes to read value");
  }
  if (value->size() > kMaxValueSize) {
    return absl::InvalidArgumentError("value size exceeds limit");
  }

  ReplyBuilder reply_builder{socket_.get()};
  Header header;
  header.message_type = MessageType::kAck;
  header.protocol_version = 1;
  header.payload_size = sizeof(uint16_t);

  absl::Status status = storage_->Put(*key, *value);

  DLOG(INFO) << "conn: put: key=" << *key << "; value=" << *value
             << "; status=" << status;

  reply_builder.WriteHeader(header);
  // Status code.
  reply_builder.WriteUint16(static_cast<uint16_t>(status.code()));
  return reply_builder.Flush();
}

absl::Status Conn::Delete(absl::Span<uint8_t> b) {
  Parser parser{b};
  absl::StatusOr<std::optional<std::string_view>> key_status =
      parser.ReadString();
  if (!key_status.ok()) {
    return key_status.status();
  }
  std::optional<std::string_view> key = *key_status;
  if (!key) {
    // We've read the whole payload so if we cannot read the requested string
    // size if means there is a protocol error.
    return absl::InvalidArgumentError("insufficient bytes to read key");
  }

  ReplyBuilder reply_builder{socket_.get()};
  Header header;
  header.message_type = MessageType::kAck;
  header.protocol_version = 1;
  header.payload_size = sizeof(uint16_t);

  absl::Status status = storage_->Delete(*key);

  DLOG(INFO) << "conn: delete: key=" << *key << "; status=" << status;

  reply_builder.WriteHeader(header);
  // Status code.
  reply_builder.WriteUint16(static_cast<uint16_t>(status.code()));
  return reply_builder.Flush();
}

}  // namespace server
}  // namespace fuddle
