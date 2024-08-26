#pragma once

#include "session-repository.h"
#include <map>
#include <queue>

class Command;
class Server;

class Session_service {
public:
  Session_service(Server *server, std::uint16_t port, std::string name);
  void start();
  void stop();

  void push_event(std::string const &player, std::string event);
  auto pop_event(std::string const &player) -> std::string;

private:
  auto on_reading_packet(Packet packet) -> Packet;

  // @return
  //  Result_type
  auto handle_player_command(std::string const &player,
                             Command command) -> std::string;

  Server *_server;
  Session_repository _session_repo;
  std::map<std::string, std::queue<std::string>> _events;
  std::string _name;
};
