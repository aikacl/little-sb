#include "application.h"
#include "client.h"
#include "server.h"
#include <thread>

auto main() -> int
{
  std::thread server_thread{[] { Server server; }};
  std::thread client_thread{[] { Client client; }};

  server_thread.detach();
  client_thread.detach();

  Application app;
  return app.run();
}
