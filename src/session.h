#pragma once

#include "sb-protocol.h"
#include <asio.hpp>
#include <print>
#include <utility>

using asio::ip::tcp;

class Seesion {
public:
  Seesion(asio::io_context &io_context,
          tcp::resolver::results_type server_endpoint,
          std::string_view const player_name)
      : _socket{io_context}, _server_endpoint{std::move(server_endpoint)},
        _player_name{player_name}
  {
    if (_ec) {
      std::println("Error: {}", _ec.message());
      return;
    }
  }

  void push_back(std::string_view const data)
  {
    _packets_list.emplace_back(Sb_packet::Sender_type::Client, _player_name,
                               data);
  }

  void send_all()
  {
    asio::connect(_socket, _server_endpoint, _ec);
    if (_ec) {
      std::println("Error: {}", _ec.message());
      return;
    }

    for (auto &data : _packets_list) {
      write(data);
    }
    _packets_list.clear();
  }

  auto receive() -> Sb_packet
  {
    std::error_code ec;
    Sb_packet response;
    _socket.read_some(asio::buffer(response), ec);
    if (ec) {
      std::println("Error: {}", ec.message());
    }
    return response;
  }

  template <typename Result_type>
  auto request(std::string_view const data) -> Result_type
  {
    push_back(data);
    send_all();
    auto const response{receive()};
    if constexpr (std::same_as<Result_type, std::string>) {
      return std::string{response.to_string()};
    }
    else {
      return *static_cast<Result_type const *>(&response);
    }
  }

private:
  void write(Sb_packet &request)
  {
    std::error_code ec;
    asio::write(_socket, asio::buffer(request), ec);
    if (ec) {
      std::println("Error: {}", ec.message());
    }
  }

  std::error_code _ec;
  tcp::socket _socket;
  tcp::resolver::results_type _server_endpoint;
  std::vector<Sb_packet> _packets_list;
  std::string _player_name;
};
