#include "server.h"

auto main() -> int
{
  asio::io_context io_context;

  Server server{io_context, 1438};

  io_context.run();

  return 0;
}
