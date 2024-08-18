#pragma once

#include <asio.hpp>
#include <string_view>

using asio::ip::tcp;

class Server {
public:
  Server(asio::io_context &io_context, std::uint16_t const server_port)
      : _acceptor{io_context, tcp::endpoint{tcp::v4(), server_port}}
  {
    tcp::socket socket{io_context};
    _acceptor.accept(socket);

    std::string_view const greeting{"Hello there!"};
    std::error_code ec;
    while (!ec) {
      asio::write(socket, asio::buffer(greeting), ec);
    }
  }

private:
  asio::ip::tcp::acceptor _acceptor;
};
