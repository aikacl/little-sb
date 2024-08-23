#include "command.h"
#include <cassert>
#include <spdlog/spdlog.h>
#include <string>

Command::Command(std::string name) : _data(json::object())
{
  _data.push_back({"name", name});
  spdlog::debug("{}", _data.dump());
  assert(_data.is_object());
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
auto Command::args() -> json
{
  return _data["args"];
}
