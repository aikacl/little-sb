#pragma once

#include "command.h"
#include "server/session-repository.h"
#include <map>
#include <queue>

class Command;
class Server;
using Event = Command;

class Session_service {
public:
  Session_service(Server *server, std::uint16_t port, std::string name);
  void start();
  void stop();

  void push_event(std::string const &player, Event event);
  void push_event_all(Event const &event);
  auto pop_event(std::string const &player) -> Event;

private:
  auto on_reading_packet(Packet packet) -> Packet;

  // @return
  //  Result_type
  auto handle_player_command(std::string const &player,
                             Command const &command) -> std::string;

  Server *_server;
  Session_repository _session_repo;
  std::map<std::string, std::queue<Event>> _events;
  std::string _name;
};
