#include <memory>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/log.h"
#include "fuddle/server/listener.h"
#include "puddle/io_uring_shard.h"
#include "puddle/listener.h"
#include "puddle/server.h"
#include "puddle/shard.h"

ABSL_FLAG(std::string, host, "127.0.0.1", "server host ip address");
ABSL_FLAG(uint16_t, port, 8119, "server port address");

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("start fuddle");
  absl::ParseCommandLine(argc, argv);

  LOG(INFO) << "fuddle: starting fuddle server; host="
            << absl::GetFlag(FLAGS_host)
            << "; port=" << absl::GetFlag(FLAGS_port);

  std::shared_ptr<puddle::Shard> shard =
      std::make_shared<puddle::IoUringShard>();

  puddle::Server server{shard};
  absl::Status listener_status =
      server.AddListener(absl::GetFlag(FLAGS_host), absl::GetFlag(FLAGS_port),
                         std::make_unique<fuddle::server::Listener>());
  if (!listener_status.ok()) {
    LOG(ERROR) << "failed to add listener; err=" << listener_status.message();
    return 1;
  }
  server.Serve();
}
