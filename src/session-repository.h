#pragma once

#include "session_fwd.h"
#include <asio.hpp>
#include <set>

using asio::ip::tcp;
class Packet;

class Session_repository {
public:
  explicit Session_repository(asio::io_context &io_context, std::uint16_t port,
                              std::function<Packet(Packet)> on_reading_packet);
  void do_accept();
  void do_close();

private:
  void on_accepting_connection(std::error_code ec, tcp::socket socket);

  asio::ip::tcp::acceptor _acceptor;
  std::set<Session_ptr> _sessions;
  std::function<Packet(Packet)> _on_reading_packet;
};
