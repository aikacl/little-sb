#include "server.h"

auto main() -> int
{
  try {
    spdlog::set_level(spdlog::level::info);
    Server server{1438};
  }
  catch (std::exception &e) {
    spdlog::error("Exception: {}", e.what());
  }

  return 0;
}
