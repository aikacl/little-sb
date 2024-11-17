#include "application.h"
#include "window.h"
#include <spdlog/spdlog.h>

auto main(int argc, char **argv) -> int
{
  spdlog::set_level(spdlog::level::info);
  Window::initialize();

#ifdef NDEBUG
  // The same as server, to trace the throwing source code.
  try {
#endif
    constexpr auto remote_host{"154.7.177.38"sv};
    constexpr auto local_host{"localhost"sv};
    constexpr auto port{"1438"sv};
    Application app{argc == 2 && std::strcmp(argv[1], "local") == 0
                        ? local_host
                        : remote_host,
                    port};
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
