#include "application.h"

auto main(int argc, char *argv[]) -> int
{
  spdlog::set_level(spdlog::level::info);
  try {
    if (argc != 2) {
      std::println("Usage: little-sb-client <player-name>");
      return 0;
    }

    Application app{"localhost", 1438, argv[1]};

    app.run();
  }
  catch (std::exception &e) {
    spdlog::error("Exception: {}", e.what());
  }

  return 0;
}
