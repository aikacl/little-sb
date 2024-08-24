#include "command.h"
#include <spdlog/spdlog.h>

Command::Command(std::string name) : _data(json::object())
{
  _data["name"] = name;
  spdlog::debug("Creating command from std::string: {}", _data.dump());
}

Command::Command(json &&data) : _data(std::move(data))
{
  spdlog::debug("Creating command from json: {}", _data.dump());
}

auto Command::name() const -> std::string
{
  return _data["name"].get<std::string>();
}

void Command::name(std::string name)
{
  _data["name"] = name;
}

auto Command::dump() const -> std::string
{
  return _data.dump();
}
auto Command::args() -> json &
{
  return _data["args"];
}

auto Command::args() const -> json const &
{
  return _data["args"];
}
