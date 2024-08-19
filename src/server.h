#pragma once

#include "game.h"
#include "handle-error.h"
#include "sb-packet.h"
#include "split-by.h"
#include <asio.hpp>
#include <map>
#include <string_view>
#include <unordered_map>

using asio::ip::tcp;

class Server {
public:
  Server(std::uint16_t const bind_port)
      : _acceptor{_io_context, tcp::endpoint{tcp::v6(), bind_port}}
  {
    run();
  }

private:
  [[nodiscard]] auto tick_interval() const
  {
    return std::chrono::milliseconds{std::chrono::seconds{1}} /
           max_tick_per_second;
  }

  void run()
  {
    process_games();
    do_accept_join();
  }

  void do_accept_join()
  {
    std::thread{[this] {
      std::println("[debug] Prepared to accept connections.");
      for (;;) {
        auto socket{_acceptor.accept()};

        auto const remote_endpoint{socket.remote_endpoint()};
        std::println("");
        std::println("[debug] Accepting new connection from {}:{}",
                     remote_endpoint.address().to_string(),
                     remote_endpoint.port());

        Sb_packet request;
        socket.read_some(asio::buffer(request), _ec);
        handle_error(_ec);

        if (request.header.protocol_name != Sb_packet::this_protocol_name) {
          std::println("Un-identified protocol: {}",
                       std::string_view{request.header.protocol_name});
          continue;
        }

        std::println("{} ({}:{}): {}", request.header.sender_name.to_string(),
                     remote_endpoint.address().to_string(),
                     remote_endpoint.port(), request.to_string());

        std::println("[debug] Preparing response");
        Sb_packet response{Sb_packet::Sender_type::Server, "Server",
                           handle_request(request)};
        std::println("Server (localhost): {}", response.to_string());
        asio::write(socket, asio::buffer(response), _ec);
        handle_error(_ec);
      }
    }}.join();
  }

  auto handle_request(Sb_packet const &request) -> std::string
  {
    if (request.header.sender_type == Sb_packet::Sender_type::Client) {
      return handle_player_request(request.header.sender_name.to_string(),
                                   request.to_string());
    }
    return std::format("Cann't parse {}'s request",
                       static_cast<int>(request.header.sender_type));
  }

  auto handle_player_request(std::string const &player_name,
                             std::string_view const command) -> std::string
  {
    auto const argv{split_by(command)};
    auto const &cmd{argv[0]};
    if (cmd == "Login") {
      // TODO(ShelpAm): add authentication.
      std::println("[debug] Log-in request from player '{}'", player_name);
      _players.insert({player_name, Player{player_name}});
      _is_player_online[player_name] = true;
      std::println("[debug] Server info updated");
      return std::format("Ok, {} logged in.", player_name);
    }
    if (cmd == "Logout") {
      _is_player_online[player_name] = false;
      return std::format("Ok, {} logged out.", player_name);
    }
    if (cmd == "list-players") {
      std::string buf;
      for (auto const &[k, _] : _is_player_online) {
        buf += k;
        buf += ' ';
      }
      return buf;
    }
    if (cmd == "battle") {
      if (argv[1] == player_name) {
        return "Can not select yourself as a component.";
      }
      if (!_is_player_online.contains(argv[1])) {
        return "Player not found.";
      }
      return std::format(
          "ok {}",
          allocate_game({_players.at(player_name), _players.at(argv[1])}).id());
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
  std::unordered_map<std::string, Player> _players;
  std::unordered_map<std::string, bool> _is_player_online;
  asio::io_context _io_context;
  tcp::acceptor _acceptor;
  std::error_code _ec;

  static constexpr std::size_t max_tick_per_second{60};
};
