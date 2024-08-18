#pragma once

#include <array>
#include <asio.hpp>
#include <iostream>

using asio::ip::tcp;

class Client {
public:
  Client()
  {
    auto const endpoints{_resolver.resolve("127.0.0.1", "")};
    asio::connect(_socket, endpoints);

    constexpr auto buffer_length{128UZ};
    std::array<char, buffer_length> buffer{};
    std::error_code ignored_error;

    while (true) {
      auto const length{_socket.read_some(asio::buffer(buffer), ignored_error)};
      std::cout.write(buffer.data(), length);
    }
  }

private:
  asio::io_context _io_context;
  tcp::socket _socket{_io_context};
  tcp::resolver _resolver{_io_context};
};
