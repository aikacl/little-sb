#pragma once

#include <source_location>
#include <spdlog/spdlog.h>
#include <system_error>

void handle_error(std::error_code const &ec)
{
  spdlog::trace("Call {}", std::source_location::current().function_name());
  if (ec) {
    throw std::runtime_error{ec.message()};
  }
}
