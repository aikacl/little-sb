#pragma once

#include "session_fwd.h"
#include <asio.hpp>
#include <map>

using asio::ip::tcp;
class Packet;

class Session_repository {
public:
  explicit Session_repository(asio::io_context &io_context, std::uint16_t port,
                              std::function<Packet(Packet)> on_read);
  void do_accept();
  void do_read(Session_ptr const &session);
  void do_close();

private:
  asio::ip::tcp::acceptor _acceptor;
  std::map<Session_ptr, std::atomic<bool>> _sessions_should_read;
  std::function<Packet(Packet)> _on_read;
};
