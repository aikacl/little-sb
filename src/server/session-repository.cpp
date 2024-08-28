#include "session-repository.h"
#include "packet.h"
#include "session.h"
#include <spdlog/spdlog.h>

Session_repository::Session_repository(asio::io_context &io_context,
                                       std::uint16_t port,
                                       std::function<Packet(Packet)> on_read)
    : _acceptor{io_context, tcp::endpoint{tcp::v6(), port}},
      _on_read{std::move(on_read)}
{
}

void Session_repository::do_accept()
{
  _acceptor.async_accept([this](std::error_code ec, tcp::socket socket) {
    spdlog::debug("Accepting new connection. {} other sessions are alive.",
                  _sessions_should_read.size());
    if (ec) {
      spdlog::warn("On accepting connection: {}", ec.message());
    }
    else {
      spdlog::trace(
          "Connection accepted, Session scheduling continuous reading...");
      auto const session{std::make_shared<Session>(std::move(socket))};
      _sessions_should_read.emplace(session, true);
      do_read(session);
    }
    do_accept();
  });
}

void Session_repository::do_read(Session_ptr const &session)
{
  session->schedule_read([this, session](Packet packet) {
    session->schedule_write(_on_read(std::move(packet)), [this, session] {
      if (_sessions_should_read[session]) {
        do_read(session);
      }
    });
  });
}

void Session_repository::do_close()
{
  for (auto &[session, should_read] : _sessions_should_read) {
    should_read = false;
  }
}
