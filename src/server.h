#pragma once

#include "game.h"
#include "sb-packet.h"
#include "session.h"
#include "split-by.h"
#include <asio.hpp>
#include <atomic>
#include <map>
#include <spdlog/spdlog.h>
#include <string_view>

using asio::ip::tcp;

class Server {
public:
  void run()
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    spdlog::info("Server started. Accepting connections...");
    do_accept();

    run_main_game_loop();
  }

  void shutdown()
  {
    spdlog::info("Server shutting down...");

    for (auto &[responding, publishing] : _responding_to_publishing) {
      responding->stop();
      publishing->stop();
    }

    _main_game_loop_should_stop = true;
  }

  static auto instance(std::uint16_t const bind_port = 0) -> Server &
  {
    static Server the_instance{bind_port};
    return the_instance;
  }

private:
  Server(std::uint16_t const bind_port)
      : _acceptor{_io_context, tcp::endpoint{tcp::v6(), bind_port}}
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
  }

  [[nodiscard]] static constexpr auto tick_interval()
  {
    return std::chrono::milliseconds{std::chrono::seconds{1}} /
           max_tick_per_second;
  }

  void do_accept()
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    _acceptor.async_accept([this](std::error_code const ec,
                                  tcp::socket socket) {
      spdlog::trace("Call {}", std::source_location::current().function_name());
      spdlog::debug("Accepted new connection; now {} other sessions are alive",
                    _publishing_session_to_name.size());
      if (!ec) {
        auto const session{std::make_shared<Session>(std::move(socket))};
        spdlog::trace("Session address in memory (accepted): {}",
                      static_cast<void const *>(session.get()));
        session->start(
            // TODO(shelpam): issue here, should I use &session instead?
            [this, session](Sb_packet const &packet) {
              spdlog::trace("Call {}",
                            std::source_location::current().function_name());

              spdlog::trace("Session address in memory (read packet): {}",
                            static_cast<void const *>(session.get()));
              auto const remote_endpoint{session->socket().remote_endpoint()};
              spdlog::info("{} ({}:{}): {}",
                           packet.header.sender.name.to_string(),
                           remote_endpoint.address().to_string(),
                           remote_endpoint.port(), packet.body.to_string());

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
                close_session(
                    _responding_to_publishing.extract(session).mapped());
                remove_player(player_name);
              }
            });
      }
      do_accept();
    });
  }

  // Only receives publishing sessions
  void close_session(Session_ptr const &session)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    auto const name{_publishing_session_to_name.extract(session).mapped()};
    spdlog::trace("Closing connection to '{}'", name);
    _publishing_name_to_session.extract(name);
  }

  void remove_player(std::string const &player_name)
  {
    _players.extract(player_name);
  }

  auto handle_packet(Session_ptr const &session,
                     Sb_packet const &packet) -> bool
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    if (packet.header.protocol_name != Sb_packet::this_protocol_name) {
      return true;
    }

    auto const &from{packet.header.sender.name.to_string()};

    switch (packet.header.sender.type) {
    case Sb_packet_sender::Type::Client:
      switch (packet.type) {
      case Sb_packet_type::Login:
        // TODO(ShelpAm): add authentication.
        if (packet.body.to_string() == "Subscribe") {
          _publishing_name_to_session.insert({from, session});
          _publishing_session_to_name.insert({session, from});
          respond(session, from, "Ok, subscribed");
          return false;
        }
        if (packet.body.to_string() == "Request") {
          _responding_to_publishing.insert(
              {session, wait_for_publishing_session(from)});
          _players.insert(
              {from, Player{from, little_sb::random::uniform(20, 25), 3}});
          respond(session, from, std::format("Ok, {} logged in.", from));
          return true;
        }
        // Unreachable
        throw std::logic_error{"Unreachable"};
      case Sb_packet_type::Message:
        respond(session, from,
                handle_player_message(from, packet.body.to_string()));
        return true;
      case Sb_packet_type::Undefined:
        respond(session, from, "Undefined Sb packet type");
        return true;
      }
    case Sb_packet_sender::Type::Administrator:
    case Sb_packet_sender::Type::Server:
    case Sb_packet_sender::Type::Undefined:
      return true;
    }
    // Unreachable
    return true;
  }

  auto
  wait_for_publishing_session(std::string const &session_name) -> Session_ptr
  {
    while (!_publishing_name_to_session.contains(session_name)) {
    }
    return _publishing_name_to_session.at(session_name);
  }

  auto handle_player_message(std::string const &player_name,
                             std::string_view const command) -> std::string
  {
    auto const argv{split_by(command)};
    auto const &cmd{argv[0]};

    if (cmd == "list-players") {
      std::string buf;
      if (argv[1] == "online") {
        for (auto const &[_, player] : _players) {
          buf += player.name();
          buf += ' ';
          buf += std::to_string(player.health());
          buf += '\n';
        }
      }
      return buf;
    }
    if (cmd == "battle") {
      if (argv[1] == player_name) {
        return "Can not select yourself as a component.";
      }
      if (!_publishing_name_to_session.contains(argv[1])) {
        return "Player not found.";
      }
      auto const game_id{
          allocate_game({&_players.at(player_name), &_players.at(argv[1])})
              .id()};
      publish(argv[1],
              std::format("You received a battle with {}", player_name));
      return std::format("ok {}", game_id);
    }
    if (cmd == "query") {
      if (argv[1] == "event") {
        auto &game{_games.at(std::stoull(argv[2]))};
        auto &events_queue{game.pending_events()};
        if (game.is_ended()) {
          _games.extract(std::stoull(argv[2]));
          return "ended";
        }
        if (events_queue.empty()) {
          return "no";
        }
        auto const event{std::move(events_queue.front())};
        events_queue.pop();
        return event;
      }
    }
    if (cmd == "damage") {
    }

    return std::format("Unrecognized command {}", cmd);
  }

  void publish(std::string const &to, std::string_view const message)
  {
    _publishing_name_to_session.at(to)->write(
        Sb_packet{Sb_packet_sender{Sb_packet_sender::Type::Server, _name},
                  Sb_packet_type::Message, message});
    spdlog::info("{}.publish->{}: {}", _name, to, message);
  }

  void respond(Session_ptr const &session, std::string_view const to,
               std::string_view const message)
  {
    session->write(
        Sb_packet{Sb_packet_sender{Sb_packet_sender::Type::Server, _name},
                  Sb_packet_type::Message, message});
    spdlog::info("{}.respond->{}: {}", _name, to, message);
  }

  auto allocate_game(std::array<Player *, 2> const &players) -> Game &
  {
    auto const id{_games.empty() ? std::uint64_t{}
                                 : _games.rbegin()->first + 1};
    return _games.insert({id, Game{id, players}}).first->second;
  }

  void run_main_game_loop()
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

  std::atomic<bool> _main_game_loop_should_stop;
  std::map<std::uint64_t, Game> _games;
  std::map<std::string, Player> _players;
  std::map<std::string, Session_ptr> _publishing_name_to_session;
  std::map<Session_ptr, std::string> _publishing_session_to_name;
  std::map<Session_ptr, Session_ptr> _responding_to_publishing;
  asio::io_context _io_context;
  tcp::acceptor _acceptor;
  std::error_code _ec;
  std::string _name{"Server"};

  static constexpr std::size_t max_tick_per_second{60};
};
