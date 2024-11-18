#include "application.h"
#include "window.h"
#include <spdlog/spdlog.h>

auto main(int argc, char **argv) -> int
{
  if (argc == 0) {
    std::abort();
  }

  if (argc != 1) {
    spdlog::error("Usage: {}   (no any argument)", *argv);
    return 1;
  }

  spdlog::set_level(spdlog::level::info);
  Window::initialize();

#ifdef NDEBUG
  // The same as server, to trace the throwing source code.
  try {
#endif
    // constexpr auto remote_host{"154.7.177.38"sv};
    // constexpr auto local_host{"localhost"sv};
    Application app;
    app.run();
#ifdef NDEBUG
  }
  catch (std::exception &e) {
    spdlog::critical("Exception: {}", e.what());
  }
#endif

  Window::deinitialize();

  return 0;
}
