#include "server.h"

auto main() -> int
{
  try {
    Server server{1438};
  }
  catch (std::exception &e) {
    spdlog::error("Exception: {}", e.what());
  }

  return 0;
}
