#pragma once

#include <system_error>

void handle_error(std::error_code const &ec)
{
  if (ec) {
    throw std::runtime_error{ec.message()};
  }
}
