#pragma once

#include "json.h"

using namespace std::literals;

class Command {
  friend void to_json(json &j, const Command &cmd);
  friend void from_json(const json &j, Command &cmd);

public:
  explicit Command(char const *name);
  explicit Command(std::string name);
  explicit Command(json data);

  [[nodiscard]] auto name() const -> std::string;
  void name(std::string name);

  template <typename T>
  [[nodiscard]] auto get_param(std::string const &key) const -> T;
  template <typename T> void set_param(std::string const &key, T value);

  // Getter and setter for arguments
  [[nodiscard]] auto args() -> json &;
  [[nodiscard]] auto args() const -> json const &;
  template <typename T>
  [[nodiscard]] auto get_arg(std::size_t index) const -> T;
  template <typename T> void add_arg(T arg);

  // Dumps command data into string.
  [[nodiscard]] auto dump() const -> std::string;

private:
  json _data;
};

template <typename T>
[[nodiscard]] auto Command::get_param(std::string const &key) const -> T
{
  return _data["params"][key].get<T>();
}

template <typename T> void Command::set_param(std::string const &key, T value)
{
  _data["params"][key] = std::move(value);
}

template <typename T>
[[nodiscard]] auto Command::get_arg(std::size_t const index) const -> T
{
  return _data["args"][index].get<T>();
}

template <typename T> void Command::add_arg(T arg)
{
  _data["args"].push_back(std::move(arg));
}
