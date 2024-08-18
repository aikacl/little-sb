#pragma once

#include <asio.hpp>
#include <string_view>

using asio::ip::tcp;

class Server {
public:
  Server() : _acceptor{_io_context, tcp::endpoint{tcp::v4(), server_port}}
  {
    tcp::socket socket{_io_context};
    _acceptor.accept(socket);

    std::string_view const greeting{"Hello there!"};
    std::error_code ignored_error;
    asio::write(socket, asio::buffer(greeting), ignored_error);
  }

private:
  std::uint16_t const server_port{1438};
  asio::io_context _io_context;
  asio::ip::tcp::acceptor _acceptor;
};
