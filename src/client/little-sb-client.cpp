#include "application.h"
#include "window.h"
#include <print>
#include <spdlog/spdlog.h>

auto main(int /*argc*/, char ** /*argv*/) -> int
{
  spdlog::set_level(spdlog::level::info);
  Window::initialize();

#ifdef NDEBUG
  // The same as server
  try {
#endif
    constexpr std::string_view server_host{"154.7.177.38"};
    // constexpr std::string_view server_host{"localhost"};
    Application app{server_host, 1438};
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
