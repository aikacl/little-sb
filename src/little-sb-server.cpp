#include <csignal>

#include "server.h"
#include <memory>
#include <spdlog/common.h>

std::unique_ptr<Server> server;

void signal_handler(int signum) {
  // Delete entire server
  server.reset(nullptr);
  exit(0);
}

auto main() -> int
{
  // Signal handler for SIGINT
  signal(SIGINT, signal_handler);
  spdlog::set_level(spdlog::level::trace);
  try {
    server = std::make_unique<Server>(1438);
    server->run();
  }
  catch (std::exception &e) {
    spdlog::error("Exception: {}", e.what());
  }

  server.reset(nullptr);
  return 0;
}
