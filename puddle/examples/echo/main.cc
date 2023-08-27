#include <memory>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/log.h"
#include "puddle/epoll_shard.h"
#include "puddle/io_uring_shard.h"
#include "puddle/listener.h"
#include "puddle/server.h"
#include "puddle/shard.h"

ABSL_FLAG(std::string, host, "127.0.0.1", "server host ip address");
ABSL_FLAG(uint16_t, port, 8119, "server port address");
ABSL_FLAG(bool, epoll, true, "use epoll instead of io_uring");

namespace echo {

class EchoListener : public puddle::Listener {
 public:
  void Connection(std::unique_ptr<puddle::Socket> conn) override;
};

void EchoListener::Connection(std::unique_ptr<puddle::Socket> conn) {
  puddle::Buffer recv_buf;
  while (true) {
    absl::StatusOr<size_t> read_n = conn->Read(&recv_buf);
    if (!read_n.ok()) {
      return;
    }
    recv_buf.Commit(*read_n);

    absl::StatusOr<size_t> write_n = conn->Write(recv_buf.committed_buf());
    if (!write_n.ok()) {
      return;
    }
    recv_buf.Consume(*read_n);
  }
}

}  // namespace echo

std::shared_ptr<puddle::Shard> CreateShard() {
  if (absl::GetFlag(FLAGS_epoll)) {
    return std::make_shared<puddle::EpollShard>();
  }
  return std::make_shared<puddle::IoUringShard>();
}

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("start the echo server");
  absl::ParseCommandLine(argc, argv);

  LOG(INFO) << "echo: starting echo server; host=" << absl::GetFlag(FLAGS_host)
            << "; port=" << absl::GetFlag(FLAGS_port)
            << "; epoll=" << absl::GetFlag(FLAGS_epoll);

  std::shared_ptr<puddle::Shard> shard = CreateShard();

  puddle::Server server{shard};
  absl::Status listener_status =
      server.AddListener(absl::GetFlag(FLAGS_host), absl::GetFlag(FLAGS_port),
                         std::make_unique<echo::EchoListener>());
  if (!listener_status.ok()) {
    LOG(ERROR) << "failed to add listener; err=" << listener_status.message();
    return 1;
  }
  server.Serve();
}
