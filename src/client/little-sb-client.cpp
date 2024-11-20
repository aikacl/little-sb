#include "application.h"
#include "log.h"
#include "window.h"

auto main(int argc, char **argv) -> int
{
  if (argc == 0) {
    std::abort();
  }

  if (argc != 1) {
    spdlog::error("Usage: {}   (no any argument)", *argv);
    return 1;
  }

  logger_common_settings();
  log_to_console_and_file(spdlog::level::warn, "logs/client.log",
                          spdlog::level::trace);
  Window::initialize();

#ifdef NDEBUG
  // The same as server, to trace the throwing source code.
  try {
#endif
    // Since application should be destructed after `Window::deinitialize()` be
    // called, we scope the construction of app.
    {
      Application app;
      app.run();
    }
#ifdef NDEBUG
  }
  catch (std::exception const &e) {
    spdlog::critical("Exception: {}", e.what());
    spdlog::dump_backtrace();
  }
#endif

  Window::deinitialize();

  return 0;
}
