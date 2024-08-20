#pragma once

#include "handle-error.h"
#include "sb-packet.h"
#include <asio.hpp>
#include <functional>
#include <memory>
#include <print>
#include <queue>
#include <source_location>
#include <spdlog/spdlog.h>

using asio::ip::tcp;

class Session;
using Session_ptr = std::shared_ptr<Session>;

class Session : std::enable_shared_from_this<Session> {
public:
  Session() = delete;

  Session(tcp::socket socket) : _socket{std::move(socket)} {}

  [[nodiscard]] auto socket() -> tcp::socket &
  {
    return _socket;
  }

  void start(std::function<void(Sb_packet const &)> const &process_packet)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    for (;;) {
      std::error_code ec;
      auto const packet{read(ec)};
      if (ec == asio::error::eof) {
        return;
      }
      spdlog::debug("Read packet: {}", packet);
      handle_error(ec);
      process_packet(packet);
    }
  }

  void push(Sb_packet const &packet)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    _packets_queue.push(packet);
  }

  void write()
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    while (!_packets_queue.empty()) {
      auto const &data{_packets_queue.front()};
      std::error_code ec;
      asio::write(_socket, asio::buffer(data), ec);
      spdlog::debug("Sent packet: {}", data);
      handle_error(ec);
      _packets_queue.pop();
    }
  }

  void write(Sb_packet const &packet)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    push(packet);
    write();
  }

  auto read() -> Sb_packet
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    Sb_packet packet;
    std::error_code ec;
    asio::read(_socket, asio::mutable_buffer(packet), ec);
    handle_error(ec);
    return packet;
  }

  auto read(std::error_code &ec) -> Sb_packet
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    Sb_packet packet;
    asio::read(_socket, asio::mutable_buffer(packet), ec);
    return packet;
  }

  template <typename Result_type>
  auto request(Sb_packet const &packet) -> Result_type
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    push(packet);
    write();
    auto const response{read()};
    if constexpr (std::same_as<Result_type, std::string>) {
      return response.to_string();
    }
    else {
      return *static_cast<Result_type const *>(&response);
    }
  }

private:
  tcp::socket _socket;
  std::queue<Sb_packet> _packets_queue;
};
