// Echo example.
//
// This example provides a simple echo server. Connect to the server with
// `nc localhost 4411`.

#include <array>
#include <csignal>
#include <iostream>

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

int main(int argc, char* argv[]) {
  // Start the Puddle runtime.
  puddle::Start();

  puddle::log::Logger logger{"main"};
  logger.Info("starting echo server; addr = {}", ":4411");

  auto listener = puddle::net::TcpListener::Bind(":4411", 128);

  puddle::NotifySignal({SIGINT, SIGTERM}, [&](int signal) {
    logger.Info("shutting down; signal = {}", strsignal(signal));
    exit(EXIT_SUCCESS);
  });

  while (true) {
    auto conn = listener.Accept();
    puddle::Spawn(Conn, std::move(conn)).Detach();
  }
}
