#include "session-service.h"
#include "command.h"
#include "game.h"
#include "packet.h"
#include "server-command-executor.h"
#include "server.h"

Session_service::Session_service(Server *server, std::uint16_t const port,
                                 std::string name)
    : _server{server},
      _session_repo{server->io_context(), port,
                    [this](Packet packet) -> Packet {
                      return on_reading_packet(std::move(packet));
                    }},
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

void Session_service::push_event(std::string const &player, std::string event)
{
  _events[player].push(std::move(event));
}

// Assume that there exitsts at least one evnet.
auto Session_service::pop_event(std::string const &player) -> std::string
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
  spdlog::debug("reply: {}", reply);
  return Packet{Packet_sender{_name, _name}, json(std::move(reply)).dump()};
}

auto Session_service::handle_player_command(std::string const &player,
                                            Command command) -> std::string
{
  spdlog::trace("Handling player's command");

  // TODO(ShelpAm): add authentication.
  if (command.name() == "login") {
    _server->_players.insert(
        {player, Player{player, little_sb::random::uniform(20, 25), 3}});
    return "Ok, you have logged in.";
  }
  if (command.name() == "battle") {
    auto const &target{command.get_arg<std::string>(0)};
    if (target == player) {
      return "Can not select yourself as a component.";
    }
    if (!_server->_players.contains(target)) {
      return "Player not found.";
    }
    auto const game_id{_server
                           ->allocate_game({&_server->_players.at(player),
                                            &_server->_players.at(target)})
                           .id()};
    Command cmd{"event"s};
    cmd.add_arg(std::format("You received a battle with {}", player));
    push_event(target, cmd.dump());
    return std::format("ok {}", game_id);
  }
  if (command.name() == "damage") {
  }
  if (command.name() == "event") {
    if (_events[player].empty()) {
      return "";
    }
    return pop_event(player);
  }
  if (command.name() == "list-players") {
    json json;
    for (auto const &[_, player] : _server->_players) {
      json.push_back(player);
    }
    return json.dump();
  }
  if (_server->_server_commands.contains(command.name())) {
    return _server->_server_commands.at(command.name())
        ->execute(player, command)
        .dump();
  }

  return std::format("Unrecognized command {}", command.name());
}
