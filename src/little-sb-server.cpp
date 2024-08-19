#include "server.h"

auto main() -> int
{
  try {
    Server server{1438};
  }
  catch (std::exception &e) {
    std::println("Exception: {}", e.what());
  }

  return 0;
}
