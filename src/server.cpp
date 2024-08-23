#include "server.h"
#include "command.h"
#include <algorithm>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

void Server::run()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  spdlog::info("Server started. Accepting connections...");
  do_accept();

  run_main_game_loop();
}

void Server::shutdown()
{
  spdlog::info("Server shutting down...");

  for (auto &[responding, publishing] : _responding_to_publishing) {
    responding->stop();
    publishing->stop();
  }

  _main_game_loop_should_stop = true;
}

auto Server::instance(std::uint16_t const bind_port) -> Server &
{
  static Server the_instance{bind_port};
  return the_instance;
}

Server::Server(std::uint16_t const bind_port)
    : _acceptor{_io_context, tcp::endpoint{tcp::v6(), bind_port}}
{
  spdlog::trace("Call {}", std::source_location::current().function_name());
}

constexpr auto Server::tick_interval()
{
  return std::chrono::milliseconds{std::chrono::seconds{1}} /
         max_tick_per_second;
}

void Server::do_accept()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  _acceptor.async_accept([this](std::error_code const ec, tcp::socket socket) {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    spdlog::debug("Accepted new connection. Now {} other sessions are alive.",
                  _publishing_session_to_name.size());
    if (!ec) {
      auto const session{std::make_shared<Session>(std::move(socket))};
      spdlog::trace("Session address in memory (accepted): {}",
                    static_cast<void const *>(session.get()));
      session->start(
          // TODO(shelpam): issue here, should I use &session instead?
          [this, session](Sb_packet const &packet) {
            return handle_packet(session, packet);
          },
          // TODO(shelpam): issue here, same as above.
          [this, session]() {
            spdlog::trace("Call {}",
                          std::source_location::current().function_name());
            if (_responding_to_publishing.contains(session)) {
              auto const &player_name{_publishing_session_to_name.at(
                  _responding_to_publishing.at(session))};
              spdlog::info("{} disconnected", player_name);
              close_session_pairs(
                  _responding_to_publishing.extract(session).mapped());
              remove_player(player_name);
            }
          });
    }
    do_accept();
  });
}

auto Server::handle_packet(Session_ptr const &session,
                           Sb_packet const &packet) -> bool
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  spdlog::debug("Session address in memory (read packet): {}",
                static_cast<void const *>(session.get()));
  auto const remote_endpoint{session->socket().remote_endpoint()};
  spdlog::debug("From {}:{}, Layout of packet: {}",
                remote_endpoint.address().to_string(), remote_endpoint.port(),
                packet);

  if (packet.protocol != Sb_packet::this_protocol_name) {
    spdlog::warn("Packet protocol is different from ours: {}", packet.protocol);
    return true;
  }

  auto const &from{packet.sender.username()};
  Command const command{json::parse(packet.payload)};

  // We must consider sign ups, but now ignore it.
  if (true || _players.contains(from)) {
    return handle_player_command(session, from, command);
  }
  // Unreachable
  throw std::logic_error{"Unreachable"};
}

auto Server::handle_player_command(Session_ptr const &session,
                                   std::string const &from,
                                   Command const &command) -> bool
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  // TODO(ShelpAm): add authentication.
  if (command.name() == "Subscribe") {
    _publishing_name_to_session.insert({from, session});
    _publishing_session_to_name.insert({session, from});
    respond(session, from, "Ok, subscribed");
    return false;
  }
  if (command.name() == "Request") {
    _responding_to_publishing.insert(
        {session, wait_for_publishing_session(from)});
    _players.insert(
        {from, Player{from, little_sb::random::uniform(20, 25), 3}});
    respond(session, from, std::format("Ok, {} logged in.", from));
    return true;
  }
  respond(session, from, parse_player_message(from, command));
  return true;
}

auto Server::parse_player_message(std::string const &player_name,
                                  Command const &command) -> std::string
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  if (command.name() == "battle") {
    auto const &opponent{command.get_arg<std::string>(0)};
    if (opponent == player_name) {
      return "Can not select yourself as a component.";
    }
    if (!_publishing_name_to_session.contains(opponent)) {
      return "Player not found.";
    }
    auto const game_id{
        allocate_game({&_players.at(player_name), &_players.at(opponent)})
            .id()};
    publish(opponent,
            std::format("You received a battle with {}", player_name));
    return std::format("ok {}", game_id);
  }
  if (command.name() == "damage") {
  }
  if (command.name() == "list-players") {
    json json;
    for (auto const &[_, player] : _players) {
      json.push_back(player);
    }
    return json.dump();
  }
  if (command.name() == "query-event") {
    auto const game_id{command.get_arg<std::int64_t>(0)};
    auto &game{_games.at(game_id)};
    if (game.ended()) {
      _games.extract(game_id);
      return "ended";
    }
    auto &events_queue{game.pending_events()};
    if (events_queue.empty()) {
      return "no";
    }
    auto const event{std::move(events_queue.front())};
    events_queue.pop();
    return event;
  }
  if (command.name() == "say") {
    auto const content{command.get_arg<std::string>(0)};
    for (auto const &[name, _] : _publishing_name_to_session) {
      publish(name, content);
    }
    return "ok";
  }

  return std::format("Unrecognized command {}", command.name());
}

void Server::close_session_pairs(Session_ptr const &session)
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  auto const name{_publishing_session_to_name.extract(session).mapped()};
  spdlog::trace("Closing connection to '{}'", name);
  _publishing_name_to_session.extract(name);
}

void Server::remove_player(std::string const &player_name)
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  _players.extract(player_name);
}

auto Server::wait_for_publishing_session(std::string const &session_name)
    -> Session_ptr
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  while (!_publishing_name_to_session.contains(session_name)) {
  }
  return _publishing_name_to_session.at(session_name);
}

void Server::publish(std::string const &to, std::string message)
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  _publishing_name_to_session.at(to)->write(Sb_packet{
      Sb_packet_sender{_name, _name}, json(std::move(message)).dump()});
  spdlog::debug("{}.publish->{}: {}", _name, to, message);
}

void Server::respond(Session_ptr const &session, std::string_view const to,
                     std::string message)
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  session->write(Sb_packet{Sb_packet_sender{_name, _name},
                           json(std::move(message)).dump()});
  spdlog::debug("{}.respond->{}: {}", _name, to, message);
}

auto Server::allocate_game(std::array<Player *, 2> const &players) -> Game &
{
  auto const id{_games.empty() ? std::uint64_t{} : _games.rbegin()->first + 1};
  return _games.insert({id, Game{id, players}}).first->second;
}

void Server::run_main_game_loop()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  auto time_since_last_update{std::chrono::steady_clock::now()};
  while (!_main_game_loop_should_stop) {
    _io_context.poll();

    std::this_thread::sleep_until(time_since_last_update + tick_interval());
    for (auto &[id, game] : _games) {
      game.tick();
    }
    time_since_last_update = std::chrono::steady_clock::now();
  }

  spdlog::trace("Main game loop over");
}
