#include <memory>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/log.h"
#include "puddle/io_uring_shard.h"
#include "puddle/listener.h"
#include "puddle/scheduler.h"
#include "puddle/server.h"
#include "puddle/shard.h"

ABSL_FLAG(std::string, host, "127.0.0.1", "server host ip address");
ABSL_FLAG(uint16_t, port, 8119, "server port address");
ABSL_FLAG(bool, connect, false, "connect to the echo server");

namespace echo {

class EchoListener : public puddle::Listener {
 public:
  void Connection(std::unique_ptr<puddle::Socket> conn) override;
};

void EchoListener::Connection(std::unique_ptr<puddle::Socket> conn) {
  LOG(INFO) << "accepted connection";

  puddle::Buffer recv_buf;
  while (true) {
    absl::StatusOr<size_t> read_n = conn->Read(recv_buf.write_buf());
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

absl::Status Server(const std::string& host, uint16_t port) {
  LOG(INFO) << "echo: starting echo server; host=" << host << "; port=" << port;

  std::shared_ptr<puddle::Shard> shard =
      std::make_shared<puddle::IoUringShard>();
  boost::fibers::use_scheduling_algorithm<puddle::Scheduler>(shard);

  puddle::Server server{shard};
  absl::Status listener_status =
      server.AddListener(host, port, std::make_unique<EchoListener>());
  if (!listener_status.ok()) {
    return listener_status;
  }
  server.Serve();
  return absl::OkStatus();
}

absl::Status Client(const std::string& host, uint16_t port) {
  LOG(INFO) << "echo: starting echo client; host=" << absl::GetFlag(FLAGS_host)
            << "; port=" << absl::GetFlag(FLAGS_port);

  std::shared_ptr<puddle::Shard> shard =
      std::make_shared<puddle::IoUringShard>();
  boost::fibers::use_scheduling_algorithm<puddle::Scheduler>(shard);

  std::unique_ptr<puddle::Socket> socket = shard->OpenSocket();
  absl::Status connect_status = socket->Connect(host, port);
  if (!connect_status.ok()) {
    return connect_status;
  }

  std::vector<uint8_t> b{1, 2, 3, 4, 5};
  absl::StatusOr<size_t> write_status = socket->Write(absl::Span<uint8_t>(b));
  if (!write_status.ok()) {
    return write_status.status();
  }

  LOG(INFO) << "written " << *write_status;

  puddle::Buffer buf;
  absl::StatusOr<size_t> read_status = socket->Read(buf.write_buf());
  if (!read_status.ok()) {
    return read_status.status();
  }

  LOG(INFO) << "read " << *read_status;

  return absl::OkStatus();
}

}  // namespace echo

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("start the echo client/server");
  absl::ParseCommandLine(argc, argv);

  if (absl::GetFlag(FLAGS_connect)) {
    absl::Status status =
        echo::Client(absl::GetFlag(FLAGS_host), absl::GetFlag(FLAGS_port));
    if (!status.ok()) {
      LOG(ERROR) << "failed to run client; err=" << status.message();
      return 1;
    }
  } else {
    absl::Status status =
        echo::Server(absl::GetFlag(FLAGS_host), absl::GetFlag(FLAGS_port));
    if (!status.ok()) {
      LOG(ERROR) << "failed to run server; err=" << status.message();
      return 1;
    }
  }
}
