#include "fuddle/server/listener.h"

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "fuddle/server/protocol.h"

namespace fuddle {
namespace server {

void Listener::Connection(std::unique_ptr<puddle::Socket> conn) {
  puddle::Buffer recv_buf;
  while (true) {
    absl::StatusOr<size_t> read_n = conn->Read(&recv_buf);
    if (!read_n.ok()) {
      return;
    }
    CHECK(*read_n > 0);
    recv_buf.Commit(*read_n);

    size_t offset;

    MessageType type;
    offset = ReadMessageType(recv_buf.committed_buf(), 0, &type);
    if (offset == 0) {
      continue;
    }

    uint16_t protocol_version;
    offset = ReadUint16(recv_buf.committed_buf(), offset, &protocol_version);
    if (offset == 0) {
      continue;
    }

    switch (type) {
      case MessageType::kPing:
        Ping(absl::Span<uint8_t>(recv_buf.committed_buf().begin() + offset,
                                 recv_buf.committed_buf().size() - offset),
             conn.get());
        break;
      default:
        // TODO(andydunstall): Unrecognised message
        break;
    }
  }
}

void Listener::Ping(absl::Span<uint8_t> b, puddle::Socket* conn) {
  std::vector<uint8_t> header(4, 0);
  header[1] = 2;
  header[3] = 1;
  absl::StatusOr<size_t> status = conn->Write(absl::Span<uint8_t>(header));
  if (!status.ok()) {
    // TODO(andydunstall)
  }
  status = conn->Write(b);
  if (!status.ok()) {
    // TODO(andydunstall)
  }
}

}  // namespace server
}  // namespace fuddle
