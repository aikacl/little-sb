#include "session-repository.h"
#include "packet.h"
#include "session.h"
#include <spdlog/spdlog.h>

Session_repository::Session_repository(
    asio::io_context &io_context, std::uint16_t const port,
    std::function<Packet(Packet)> on_read_packet,
    std::function<void()> post_session_end)
    : _acceptor{io_context, tcp::endpoint{tcp::v6(), port}},
      _on_read_packet{std::move(on_read_packet)},
      _post_session_end{std::move(post_session_end)}
{
}

void Session_repository::do_accept()
{
  _acceptor.async_accept([this](std::error_code ec, tcp::socket socket) {
    on_accepting_connection(ec, std::move(socket));
  });
}

void Session_repository::do_close()
{
  for (auto const &session : _sessions) {
    session->stop();
  }
}

void Session_repository::on_accepting_connection(std::error_code ec,
                                                 tcp::socket socket)
{
  spdlog::debug("Accepting new connection. {} other sessions are alive.",
                _sessions.size());
  if (!ec) {
    auto const session{std::make_shared<Session>(std::move(socket))};
    _sessions.insert(session);
    spdlog::trace("Accepted session address in memory: {}",
                  static_cast<void const *>(session.get()));
    // TODO(shelpam): issue here, should I use &session instead?
    session->start(_on_read_packet, _post_session_end);
  }
  do_accept();
}
