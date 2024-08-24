#pragma once

#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;
using namespace std::literals;

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
  template <typename T> void set_param(std::string const &key, T value)
  {
    _data["params"][key] = std::move(value);
  }

  auto args() -> json &;
  [[nodiscard]] auto args() const -> json const &;
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

class Server;
class Server_command_executor {
public:
  // We should keep guarantee that `receiver` won't be released
  Server_command_executor(Server *server);
  virtual ~Server_command_executor() = 0;
  virtual auto execute(std::string from, Command const &command) -> json = 0;
  virtual constexpr auto name() -> std::string = 0;
  auto server() -> Server *;

private:
  Server *_server;
};

class Say_server_command_executor : public Server_command_executor {
public:
  Say_server_command_executor(Server *server);
  auto execute(std::string from, Command const &command) -> json final;
  constexpr auto name() -> std::string final;
};

class Query_event_server_command_executor : public Server_command_executor {
public:
  Query_event_server_command_executor(Server *server);
  auto execute(std::string from, Command const &command) -> json final;
  constexpr auto name() -> std::string final;
};
