#pragma once

#include <spdlog/spdlog.h>

void logger_common_settings();

void log_to_console_and_file(spdlog::level::level_enum console_level,
                             std::string const &filename,
                             spdlog::level::level_enum file_level);
