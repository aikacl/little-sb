#include "server.h"
#include <thread>

auto main() -> int
{
  try {
    asio::io_context io_context;

    Server server{io_context, 1438};

    std::thread server_thread{[&io_context] { io_context.run(); }};
    server_thread.detach();
  }
  catch (std::exception &e) {
    std::println("Exception: {}", e.what());
  }

  return 0;
}
