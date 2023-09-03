#include <memory>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/log.h"
#include "otter/server/listener.h"
#include "otter/storage/inmemory_storage.h"
#include "otter/storage/storage.h"
#include "puddle/io_uring_shard.h"
#include "puddle/listener.h"
#include "puddle/server.h"
#include "puddle/shard.h"

ABSL_FLAG(std::string, host, "127.0.0.1", "server host ip address");
ABSL_FLAG(uint16_t, port, 8119, "server port address");

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage("start otter");
  absl::ParseCommandLine(argc, argv);

  LOG(INFO) << "otter: starting otter server; host="
            << absl::GetFlag(FLAGS_host)
            << "; port=" << absl::GetFlag(FLAGS_port);

  std::shared_ptr<puddle::Shard> shard =
      std::make_shared<puddle::IoUringShard>();

  LOG(INFO) << "otter: opened rocksdb storage; path=./data";

  std::shared_ptr<otter::storage::Storage> storage =
      std::make_shared<otter::storage::InMemoryStorage>();

  puddle::Server server{shard};
  absl::Status listener_status =
      server.AddListener(absl::GetFlag(FLAGS_host), absl::GetFlag(FLAGS_port),
                         std::make_unique<otter::server::Listener>(storage));
  if (!listener_status.ok()) {
    LOG(ERROR) << "failed to add listener; err=" << listener_status.message();
    return 1;
  }
  server.Serve();
}
