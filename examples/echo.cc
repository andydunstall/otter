// Echo example.
//
// This example provides a simple echo server. Connect to the server with
// `nc localhost 4411`.

#include <array>
#include <csignal>
#include <iostream>

#include "puddle/cli/command.h"
#include "puddle/log/log.h"
#include "puddle/net/tcp.h"
#include "puddle/puddle.h"
#include "puddle/signal.h"

void Conn(puddle::net::TcpConn conn) {
  std::array<uint8_t, 256> buf;
  while (true) {
    try {
      size_t read_n = conn.Read(buf.data(), buf.size());
      if (read_n == 0) {
        return;
      }

      // Echo the read bytes (which may require multiple writes).
      size_t write_n = 0;
      while (write_n < read_n) {
        write_n += conn.Write(buf.data() + write_n, read_n - write_n);
      }
    } catch (const std::exception& e) {
      std::cout << "client error: " << e.what() << std::endl;
    }
  }
}

void Server(const std::string& addr, puddle::Config config) {
  // Start the Puddle runtime.
  puddle::Start(config);

  puddle::log::Logger logger{"main"};
  logger.Info("starting echo server; addr = {}", addr);

  auto listener = puddle::net::TcpListener::Bind(addr, 128);

  puddle::NotifySignal({SIGINT, SIGTERM}, [&](int signal) {
    logger.Info("shutting down; signal = {}", strsignal(signal));
    exit(EXIT_SUCCESS);
  });

  while (true) {
    auto conn = listener.Accept();
    puddle::Spawn(Conn, std::move(conn)).Detach();
  }
}

int main(int argc, char* argv[]) {
  puddle::cli::Flags flags;

  std::string addr = ":4411";
  flags.Add<std::string>("addr", &addr, "Server listen address.");

  puddle::Config config = puddle::Config::Default();
  flags.Add<int>("runtime.ring-size", &config.reactor.ring_size,
                 "Runtime io_uring ring size.");
  flags.Add<puddle::log::Level>("log.level", &config.log.level,
                                "Log level (error, warn, info, debug, trace).");

  flags.Parse(argc, argv);

  if (flags.help()) {
    fmt::print(R"(Echo server example.

Start the server with:

  $ echo

Which will listen on port 4411 by default, or you can configure the listen
address with the `--addr` flag:

$   echo --addr ":5511"

See `echo -h` for details.
)");

    return EXIT_SUCCESS;
  }

  Server(addr, config);
return EXIT_SUCCESS;
}
