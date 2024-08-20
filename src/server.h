#pragma once

#include "game.h"
#include "sb-packet.h"
#include "session.h"
#include "split-by.h"
#include <asio.hpp>
#include <map>
#include <spdlog/spdlog.h>
#include <string_view>

using asio::ip::tcp;

class Server {
public:
  Server(std::uint16_t const bind_port)
      : _acceptor{_io_context, tcp::endpoint{tcp::v6(), bind_port}}
  {
  }

  void run()
  {
    process_games();
    do_accept();
  }
private:
  [[nodiscard]] static constexpr auto tick_interval()
  {
    return std::chrono::milliseconds{std::chrono::seconds{1}} /
           max_tick_per_second;
  }

  void do_accept()
  {
    spdlog::info("Server started");
    for (std::size_t i{};; ++i) {
      std::thread{
          [this, i](Session_ptr const &session) {
            spdlog::debug(
                "Accepting new connection; this is {}-th acceptance, and "
                "now {} sessions are alive",
                i, _publishing_session_to_name.size());
            session->start([this, &session](Sb_packet const &packet) {
              auto const remote_endpoint{session->socket().remote_endpoint()};
              spdlog::info("{} ({}:{}): {}",
                           packet.header.sender.name.to_string(),
                           remote_endpoint.address().to_string(),
                           remote_endpoint.port(), packet.body.to_string());

              return handle_packet(session, packet);
            });
            if (_responding_to_publishing.contains(session)) {
              spdlog::info("{} disconnected",
                           _publishing_session_to_name.at(
                               _responding_to_publishing.at(session)));
              close_session(
                  _responding_to_publishing.extract(session).mapped());
            }
          },
          std::make_shared<Session>(_acceptor.accept())}
          .detach(); // This idiom keeps Session in live in the whole thread
    }
  }

  // Only receives publishing sessions
  void close_session(Session_ptr const &session)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    auto const name{_publishing_session_to_name.extract(session).mapped()};
    spdlog::trace("Closing connection to '{}'", name);
    _publishing_name_to_session.extract(name);
  }

  auto handle_packet(Session_ptr const &session,
                     Sb_packet const &packet) -> bool
  {
    if (packet.header.protocol_name != Sb_packet::this_protocol_name) {
      return true;
    }

    if (packet.header.sender.type == Sb_packet_sender::Type::Client) {
      auto const &from{packet.header.sender.name.to_string()};
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
          // Wait for this
          for (; !_publishing_name_to_session.contains(from);) {
          }
          _responding_to_publishing.insert(
              {session, _publishing_name_to_session.at(from)});
          respond(session, from, std::format("Ok, {} logged in.", from));
        }
        _players.insert({from, Player{from}});
        return true;
      case Sb_packet_type::Message:
        respond(session, from,
                handle_player_message(from, packet.body.to_string()));
        return true;
      case Sb_packet_type::Undefined:
        respond(session, from, "Undefined Sb packet type");
        return true;
      }
    }
    return true;
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

  auto handle_player_message(std::string const &player_name,
                             std::string_view const command) -> std::string
  {
    auto const argv{split_by(command)};
    auto const &cmd{argv[0]};

    if (cmd == "list-players") {
      std::string buf;
      if (argv[1] == "online") {
        for (auto const &[k, _] : _publishing_name_to_session) {
          buf += k;
          buf += ' ';
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
          allocate_game({_players.at(player_name), _players.at(argv[1])}).id()};
      publish(argv[1],
              std::format("You received a battle with {}", player_name));
      return std::format("ok {}", game_id);
    }
    if (cmd == "query") {
      if (argv[1] == "event") {
        auto &game{_games.at(std::stoull(argv[2]))};
        auto &events_queue{game.pending_events()};
        if (events_queue.empty()) {
          if (game.is_ended()) {
            return "ended";
          }
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

  auto allocate_game(std::array<Player, 2> const &players) -> Game &
  {
    auto const id{_games.empty() ? std::uint64_t{}
                                 : _games.rbegin()->first + 1};
    return _games.insert({id, Game{id, players}}).first->second;
  }

  void process_games()
  {
    std::thread{[this] {
      auto time_since_last_update{std::chrono::steady_clock::now()};
      for (;;) {
        std::this_thread::sleep_until(time_since_last_update + tick_interval());
        for (auto &[id, game] : _games) {
          game.tick();
        }
        time_since_last_update = std::chrono::steady_clock::now();
      }
    }}.detach();
  }

  std::map<std::uint64_t, Game> _games;
  std::map<std::string, Player> _players;
  std::map<std::string, Session_ptr> _publishing_name_to_session;
  std::map<Session_ptr, std::string> _publishing_session_to_name;
  std::map<Session_ptr, Session_ptr> _responding_to_publishing;
  asio::io_context _io_context;
  tcp::acceptor _acceptor;
  std::error_code _ec;
  std::string_view _name{"Server"};

  static constexpr std::size_t max_tick_per_second{60};
};
