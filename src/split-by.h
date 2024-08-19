#pragma once

#include <ranges>
#include <string_view>
#include <vector>

auto split_by(std::string_view const cmd,
              char delim = ' ') -> std::vector<std::string>
{
  std::vector<std::string> parts;
  for (auto const &part : cmd | std::views::split(delim)) {
    parts.emplace_back(std::string_view{part});
  }
  return parts;
}
