#pragma once

#include "event.h"

class Command;
class Server;

class Server_command_executor {
public:
  // We should keep guarantee that `receiver` won't be released
  Server_command_executor(Server *server);
  virtual ~Server_command_executor() = 0;
  virtual auto execute(std::string from, Command const &command) -> Event = 0;
  virtual constexpr auto name() -> std::string = 0;
  auto server() -> Server *;

private:
  Server *_server;
};

class Say_server_command_executor : public Server_command_executor {
public:
  Say_server_command_executor(Server *server);
  auto execute(std::string from, Command const &command) -> Event final;
  constexpr auto name() -> std::string final;
};

// Move code in session-service.cpp here. This is temporarily unused.
class Query_event_server_command_executor : public Server_command_executor {
public:
  Query_event_server_command_executor(Server *server);
  auto execute(std::string from, Command const &command) -> Event final;
  constexpr auto name() -> std::string final;
};

class Fuck_server_command_executor : public Server_command_executor {
public:
  Fuck_server_command_executor(Server *server);
  auto execute(std::string from, Command const &command) -> Event final;
  constexpr auto name() -> std::string final;
};
