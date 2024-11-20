#include "log.h"
#include "server.h"
#include <csignal>
#include <memory>
#include <span>

void signal_handler(int /*signal*/)
{
  // Delete entire server
  Server::instance().shutdown();
}

auto main(int argc, char **argv) -> int
{
  if (argc == 0) {
    std::abort();
    return -1;
  }

  std::span args{argv, static_cast<std::size_t>(argc - 1)};

  // Signal handler for SIGINT
  std::signal(SIGINT, signal_handler);
  log_to_console_and_file(spdlog::level::info, "logs/server.log",
                          spdlog::level::trace);

#ifdef NDEBUG
  // Disable try-catch in DEBUG mode to allow the debugger to catch and display
  // the original exception location, rather than catching it here and losing
  // the call stack information.
  try {
#endif
    Server::instance(1438).run();
#ifdef NDEBUG
  }
  catch (std::exception const &e) {
    spdlog::critical("Exception: {}", e.what());
    spdlog::dump_backtrace();
  }
#endif

  return 0;
}
