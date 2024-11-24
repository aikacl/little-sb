#pragma once

#include "event.h"

class Command;
class Server;

class Server_command_executor {
public:
  Server_command_executor(Server *server);
  Server_command_executor(const Server_command_executor &) = delete;
  Server_command_executor(Server_command_executor &&) = delete;
  auto operator=(const Server_command_executor &)
      -> Server_command_executor & = delete;
  auto operator=(Server_command_executor &&)
      -> Server_command_executor & = delete;
  virtual ~Server_command_executor() = default;
  virtual auto execute(std::string from, Command const &command) -> Event = 0;

  // Since constexpr implies inline, it should be inline in derived classes.
  virtual constexpr auto name() -> std::string = 0;

  auto server() -> Server *;

private:
  Server *_server;
};

class Say_server_command_executor : public Server_command_executor {
public:
  Say_server_command_executor(Server *server);
  auto execute(std::string from, Command const &command) -> Event final;
  constexpr auto name() -> std::string final
  {
    return "say"s;
  }
};

// Move code in session-service.cpp here. This is temporarily unused.
class Query_event_server_command_executor : public Server_command_executor {
public:
  Query_event_server_command_executor(Server *server);
  auto execute(std::string from, Command const &command) -> Event final;
  constexpr auto name() -> std::string final
  {
    return "query-event"s;
  }
};

class Fuck_server_command_executor : public Server_command_executor {
public:
  Fuck_server_command_executor(Server *server);
  auto execute(std::string from, Command const &command) -> Event final;
  constexpr auto name() -> std::string final
  {
    return "fuck"s;
  }
};

class Escape_server_command_executor : public Server_command_executor {
public:
  Escape_server_command_executor(Server *server);
  auto execute(std::string from, Command const &command) -> Event final;
  constexpr auto name() -> std::string final
  {
    return "escape"s;
  }
};

namespace server_command_executors {

class Resurrect : public Server_command_executor {
public:
  Resurrect(Server *server);
  auto execute(std::string from, Command const &command) -> Event final;
  constexpr auto name() -> std::string final
  {
    return "resurrect"s;
  }
};

} // namespace server_command_executors
