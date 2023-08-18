#include <memory>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/log.h"
#include "puddle/epoll_shard.h"
#include "puddle/listener.h"
#include "puddle/server.h"
#include "puddle/shard.h"

ABSL_FLAG(std::string, host, "127.0.0.1", "server host ip address");
ABSL_FLAG(uint16_t, port, 8119, "server port address");

namespace echo {

class EchoListener : public puddle::Listener {
 public:
  void Connection(std::unique_ptr<puddle::Conn> conn) override;
};

void EchoListener::Connection(std::unique_ptr<puddle::Conn> conn) {}

}  // namespace echo

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("start the echo server");
  absl::ParseCommandLine(argc, argv);

  LOG(INFO) << "echo: starting echo server; host=" << absl::GetFlag(FLAGS_host)
            << "; port=" << absl::GetFlag(FLAGS_port);

  std::shared_ptr<puddle::Shard> shard = std::make_shared<puddle::EpollShard>();

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
