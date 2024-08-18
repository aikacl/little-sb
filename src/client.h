#pragma once

#include <array>
#include <asio.hpp>
#include <iostream>
#include <print>

using asio::ip::tcp;

class Client {
public:
  Client(asio::io_context &io_context, std::string_view const host,
         std::uint16_t const port)
      : _socket{io_context}, _resolver{io_context}
  {
    std::error_code ec;

    auto const endpoints{
        _resolver.resolve(tcp::v4(), host, std::to_string(port))};
    asio::connect(_socket, endpoints, ec);
    if (ec) {
      std::println("Error: {}", ec.message());
      return;
    }

    constexpr auto buffer_length{128};
    std::array<char, buffer_length> buffer{};

    while (ec != asio::error::eof) {
      auto const length{_socket.read_some(asio::buffer(buffer), ec)};
      std::cout.write(buffer.data(), length);
    }
  }

private:
  tcp::socket _socket;
  tcp::resolver _resolver;
};
