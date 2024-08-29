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
  Command const player_command{json::parse(std::move(packet.payload))};
  auto reply{handle_player_command(packet.sender.username(), player_command)};
  return Packet{Packet_sender{_name, _name}, reply.dump()};
}

auto Session_service::handle_player_command(std::string const &player_name,
                                            Command const &command) -> Event
{
  spdlog::trace("Handling player's command");

  // TODO(ShelpAm): add authentication.
  if (command.name() == "login") {
    _server->_players.insert(
        {player_name, Player{player_name, little_sb::random::uniform(20, 25),
                             little_sb::random::uniform(3, 7),
                             little_sb::random::uniform(1, 5), 20}});
    spdlog::info("{} logged in", player_name);
    // When logged in, the player had upload its data, and we create new
    // information if the player instance doesn't exist. So here the player
    // must exist.
    Event e{"ok"};
    e.add_arg(_server->_players[player_name]);
    return e;
  }

  auto &player{_server->_players.at(player_name)};

  if (command.name() == "battle") {
    auto const &target{command.get_arg<std::string>(0)};
    spdlog::debug("target: {}, player: {}", target, player_name);
    if (target == player_name) {
      Event e{"error"s};
      e.add_arg("Can not select yourself as a component.");
      return e;
    }
    if (!_server->_players.contains(target)) {
      Event e{"error"s};
      e.add_arg("Player not found.");
      return e;
    }
    auto const &game{_server->allocate_game(
        {&_server->_players[player_name], &_server->_players[target]})};
    Event battle{"battle"s};
    battle.set_param("from", player_name);
    push_event(target, battle);
    Event e{"ok"s};
    e.set_param("game-id", game.id());
    return e;
  }
  if (command.name() == "buy") {
    auto const item_name{command.get_arg<std::string>(0)};
    auto const &item{_server->_store_items[item_name]};
    if (player.money() < item.price) {
      Event e{"error"};
      e.add_arg("You don't have enough money to buy this item!");
      return e;
    }
    player.cost_money(item.price);
    // item.effect;
    // TODO(shelpam): now only provides one goods, so not using flexible way to
    // achieve the effect.
    if (item.name == "First aid kit") {
      player.cure(10);
      Event cure{"cure"};
      cure.add_arg(10);
      cure.set_param("cause",
                     "You bought First aid kit, health increase by 10");
      push_event(player_name, cure);
    }
    return Event{"ok"};
  }
  if (command.name() == "list-store-items") {
    Event e{"ok"};
    e.add_arg(_server->_store_items);
    return e;
  }
  if (command.name() == "list-players") {
    Event e{"ok"};
    e.add_arg(_server->_players);
    return e;
  }
  if (command.name() == "query-event") {
    if (_events[player_name].empty()) {
      push_event(player_name, Event("none"s));
    }
    return pop_event(player_name);
  }
  if (command.name() == "sync") {
    Event e{"ok"};
    e.add_arg(player);
    return e;
  }

  if (_server->_server_commands.contains(command.name())) {
    return _server->_server_commands.at(command.name())
        ->execute(player_name, command);
  }

  Event e{"error"};
  e.add_arg(std::format("Unrecognized command {}", command.name()));
  return e;
}
