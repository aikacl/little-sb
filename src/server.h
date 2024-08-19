#pragma once

#include "handle-error.h"
#include "player.h"
#include "sb-packet.h"
#include <asio.hpp>
#include <ranges>
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
  void run()
  {
    std::println("Prepared to accept connections.");
    while (true) {
      auto socket{_acceptor.accept()};

      auto const remote_endpoint{socket.remote_endpoint()};
      std::println("");
      std::println("Accepting new connection from {}:{}",
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

      std::println("Request message: {}", request.to_string());

      std::println("Preparing response");
      Sb_packet response{Sb_packet::Sender_type::Server, "Server",
                         handle_request(request)};
      std::println("Responding: {}", response.to_string());
      asio::write(socket, asio::buffer(response), _ec);
      handle_error(_ec);
    }
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

  static auto split_by(std::string_view const cmd,
                       char delim = ' ') -> std::vector<std::string>
  {
    std::vector<std::string> parts;
    for (auto const &part : cmd | std::views::split(delim)) {
      parts.emplace_back(std::string_view{part});
    }
    return parts;
  }

  auto handle_player_request(std::string_view const player_name,
                             std::string_view const cmd) -> std::string
  {
    if (cmd == "Login") {
      // TODO(ShelpAm): add authentication.
      std::println("Log-in request from player '{}'", player_name);
      _players.try_emplace(std::string{player_name}, player_name);
      _online[std::string{player_name}] = true;
      std::println("Server info updated");
      return std::format("Ok, {} logged in.", player_name);
    }
    if (cmd == "Logout") {
      _online[std::string{cmd}] = false;
      return std::format("Ok, {} logged out.", player_name);
    }
    if (cmd == "list-players") {
      return "I don't know how to implement this";
    }
    if (cmd == "damage") {
    }

    return "Unspecified";
  }

  std::unordered_map<std::string, Player> _players;
  std::unordered_map<std::string, bool> _online;
  asio::io_context _io_context;
  tcp::acceptor _acceptor;
  std::error_code _ec;
};
