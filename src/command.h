#pragma once

#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;

class Command {
public:
  explicit Command(std::string name);
  explicit Command(json &&data);
  [[nodiscard]] auto name() const -> std::string;
  void name(std::string name);

  template <typename T>
  [[nodiscard]] auto get_param(std::string const &key) const -> T
  {
    return _data["params"][key].get<T>();
  }
  template <typename T> void set_param(std::string const &key, T &&value)
  {
    _data["params"][key] = std::forward(value);
  }

  auto args() -> json;
  template <typename T>
  [[nodiscard]] auto get_arg(std::size_t const index) const -> T
  {
    return _data["args"][index].get<T>();
  }
  template <typename T> void add_arg(T arg)
  {
    _data["args"].push_back(std::move(arg));
  }

  [[nodiscard]] auto dump() const -> std::string;

private:
  json _data;
};
