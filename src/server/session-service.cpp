#include "session-service.h"
#include "command.h"
#include "game.h"
#include "packet.h"
#include "random.h"
#include "server-command-executor.h"
#include "server.h"

Session_service::Session_service(Server *server, std::uint16_t const port,
                                 std::string name)
    : _server{server},
      _session_repo{server->io_context(), port,
                    [this](Packet packet) -> Packet {
                      return on_reading_packet(std::move(packet));
                    },
                    []() {}},
      _name{std::move(name)}
{
}

void Session_service::start()
{
  _session_repo.do_accept();
}

void Session_service::stop()
{
  _session_repo.do_close();
}

void Session_service::push_event(std::string const &player, Event event)
{
  _events[player].push(std::move(event));
}

void Session_service::push_event_all(Event const &event)
{
  for (auto &[_, queue] : _events) {
    queue.push(event);
  }
}

// Assume that there exitsts at least one evnet.
auto Session_service::pop_event(std::string const &player) -> Event
{
  auto event{std::move(_events[player].front())};
  _events[player].pop();
  return event;
}

auto Session_service::on_reading_packet(Packet packet) -> Packet
{
  if (packet.protocol != Packet::this_protocol_name) {
    spdlog::warn("Packet protocol is different from ours: {}", packet.protocol);
    json reply("Protocol error: {} used.");
    return Packet{Packet_sender{"Server", "Server"}, reply.dump()};
  }

  if (!_server->verify_userinfo(packet.sender)) {
    return Packet{Packet_sender{_name, _name},
                  json("Wrong username or password.").dump()};
  }

  // We must consider sign-ups, but for now just ignore it.
  auto reply{
      handle_player_command(packet.sender.username(),
                            Command{json::parse(std::move(packet.payload))})};
  spdlog::debug("Replying: {}", reply);
  return Packet{Packet_sender{_name, _name}, json(std::move(reply)).dump()};
}

auto Session_service::handle_player_command(
    std::string const &player, Command const &command) -> std::string
{
  spdlog::trace("Handling player's command");

  // TODO(ShelpAm): add authentication.
  if (command.name() == "login") {
    _server->_players.insert(
        {player, Player{player, little_sb::random::uniform(20, 25),
                        little_sb::random::uniform(3, 7),
                        little_sb::random::uniform(1, 5)}});
    return "Ok, you have logged in.";
  }
  if (command.name() == "battle") {
    auto const &target{command.get_arg<std::string>(0)};
    if (target == player) {
      Event error{"error"s};
      error.add_arg("Can not select yourself as a component.");
      return error.dump();
    }
    if (!_server->_players.contains(target)) {
      Event error{"error"s};
      error.add_arg("Player not found.");
      return error.dump();
    }
    auto const &game{_server->allocate_game(
        {&_server->_players[player], &_server->_players[target]})};
    Event battle{"battle"s};
    battle.set_param("from", player);
    push_event(target, battle);
    Command ok{"ok"s};
    ok.set_param("game-id", game.id());
    return ok.dump();
  }
  if (command.name() == "damage") {
  }
  if (command.name() == "query-event") {
    if (_events[player].empty()) {
      push_event(player, Event("none"s));
    }
    return pop_event(player).dump();
  }
  if (command.name() == "list-players") {
    return json(_server->_players).dump();
  }
  if (command.name() == "query-player") {
    // When logged in, the player had upload its data, and we create new
    // information if the player instance doesn't exist. So here the player
    // must exist.
    return json(_server->_players[player]).dump();
  }
  if (_server->_server_commands.contains(command.name())) {
    return _server->_server_commands.at(command.name())
        ->execute(player, command)
        .dump();
  }

  return std::format("Unrecognized command {}", command.name());
}
