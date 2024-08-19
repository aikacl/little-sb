#include "application.h"
#include "session.h"

auto main() -> int
{
  asio::io_context io_context;

  auto const server_endpoints{
      tcp::resolver{io_context}.resolve("localhost", "1438")};
  Seesion session{io_context, server_endpoints, "You"};

  Application app{&session};
  auto const execution_result{app.run()};

  io_context.run();

  return execution_result;
}
