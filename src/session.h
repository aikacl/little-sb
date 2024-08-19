#pragma once

#include "handle-error.h"
#include "sb-packet.h"
#include <asio.hpp>
#include <print>

using asio::ip::tcp;

class Session {
public:
  Session(asio::io_context &io_context,
          tcp::resolver::results_type server_endpoints,
          std::string_view const player_name)
      : _socket{io_context}, _server_endpoints{std::move(server_endpoints)},
        _player_name{player_name}
  {
    handle_error(_ec);
  }

  void push_back(std::string_view const data)
  {
    _packets_list.emplace_back(Sb_packet::Sender_type::Client, _player_name,
                               data);
  }

  void send_all()
  {
    asio::connect(_socket, _server_endpoints, _ec);
    handle_error(_ec);

    for (auto &data : _packets_list) {
      write(data);
    }
    _packets_list.clear();
  }

  auto receive() -> Sb_packet
  {
    Sb_packet response;
    _socket.read_some(asio::buffer(response), _ec);
    handle_error(_ec);

    return response;
  }

  template <typename Result_type>
  auto request(std::string_view const data) -> Result_type
  {
    push_back(data);
    send_all();
    auto const response{receive()};
    if constexpr (std::same_as<Result_type, std::string>) {
      return response.to_string();
    }
    else {
      return *static_cast<Result_type const *>(&response);
    }
  }

  [[nodiscard]] auto player_name() const -> std::string_view
  {
    return _player_name;
  }

private:
  void write(Sb_packet &request)
  {
    asio::write(_socket, asio::buffer(request), _ec);
    handle_error(_ec);
  }

  std::error_code _ec;
  tcp::socket _socket;
  tcp::resolver::results_type _server_endpoints;
  std::vector<Sb_packet> _packets_list;
  std::string _player_name;
};
