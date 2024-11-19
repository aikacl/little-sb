#include "log.h"
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

void log_to_console_and_file()
{
  auto console_sink{std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
  console_sink->set_level(spdlog::level::warn);

  auto file_sink{std::make_shared<spdlog::sinks::basic_file_sink_mt>(
      "logs/multisink.txt", true)};
  file_sink->set_level(spdlog::level::trace);

  auto logger{std::make_shared<spdlog::logger>(
      "", spdlog::sinks_init_list{console_sink, file_sink})};

  spdlog::default_logger()->name();
  spdlog::set_default_logger(logger);
}
