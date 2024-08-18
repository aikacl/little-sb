#include "client.h"
#include "application.h"

auto main() -> int
{
  asio::io_context io_context;

  Client client{io_context, "127.0.0.1", 1438};

  io_context.run();

  return 0;

  Application app;
  return app.run();
}
