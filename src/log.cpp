#include "log.h"
#include <algorithm>
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

void logger_common_settings()
{
  spdlog::enable_backtrace(64);
  spdlog::flush_every(std::chrono::seconds(3));
}

void log_to_console_and_file(spdlog::level::level_enum console_level,
                             std::string const &filename,
                             spdlog::level::level_enum file_level)
{
  auto console_sink{std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
  console_sink->set_level(console_level);

  auto file_sink{
      std::make_shared<spdlog::sinks::basic_file_sink_mt>(filename, true)};
  file_sink->set_level(file_level);

  auto logger{std::make_shared<spdlog::logger>(
      "", spdlog::sinks_init_list{console_sink, file_sink})};
  logger->set_level(std::min(console_level, file_level));

  spdlog::set_default_logger(logger);
}
