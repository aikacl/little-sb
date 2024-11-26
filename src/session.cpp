#include "session.h"
#include <source_location>

Session::Session(tcp::socket socket) : _socket{std::move(socket)} {}

void Session::schedule_request(Packet packet,
                               std::function<void(Packet)> on_replied)
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  schedule_write(std::move(packet), [self{shared_from_this()},
                                     on_read{std::move(on_replied)}]() mutable {
    self->schedule_read(std::move(on_read));
  });
}

void Session::schedule_read(std::function<void(Packet)> on_read)
{
  auto const read_not_in_progress{_pending_reads.empty()};

  _pending_reads.push(std::move(on_read));

  if (read_not_in_progress) {
    do_async_read();
  }
}

void Session::schedule_write(Packet packet, std::function<void()> post_write)
{
  auto const write_not_in_progress{_pending_writes.empty()};

  _pending_writes.emplace(std::move(packet), std::move(post_write));

  if (write_not_in_progress) {
    do_async_write();
  }
}

void Session::do_async_read()
{
  asio::async_read_until(
      _socket,
      // _streambuf,
      asio::dynamic_buffer(_read_buf), '\n',
      // Captures self by value to extend session's lifetime
      [self{shared_from_this()}](std::error_code ec, std::size_t length) {
        if (ec == asio::error::eof) {
          return;
        }
        if (ec) {
          // Do not throw exception because client can construct a malformed
          // package, or just not responding to us. Throwing exception can
          // cause server stop.
          spdlog::warn("Error occurred while reading from session: {}, closing "
                       "this session.",
                       ec.message());
          return;
        }

        // std::istream is{&self->_streambuf};
        // std::stringstream ss;
        // ss << is.rdbuf();
        // spdlog::debug("The buffer now: {}", ss.str());
        spdlog::debug("The buffer now: BUFFER_BEGIN\n{}\nBUFFER_END",
                      self->_read_buf);
        std::string buffer;
        // std::getline(ss, buffer);
        // TODO(shelpam): this is slow; improve this. Maybe use
        // asio::streambuf.
        buffer = self->_read_buf.substr(0, length - 1);
        self->_read_buf = self->_read_buf.substr(length);
        spdlog::debug("Read bytes: {}", buffer);
        spdlog::debug("Read bytes Length: {}", length);
        auto packet{json::parse(buffer).get<Packet>()};
        spdlog::debug("Reading from socket: {}", packet);

        // Same as above
        try {
          self->_pending_reads.front()(std::move(packet));
          self->_pending_reads.pop();
          if (!self->_pending_reads.empty()) {
            self->do_async_read();
          }
        }
        catch (std::runtime_error &re) {
          spdlog::error("Error occurred: {}, closing this session.", re.what());
        }
      });
}

void Session::do_async_write()
{
  spdlog::debug("PENDING WRITES QUEUE SIZE: {}", _pending_writes.size());

  _write_buf = json(_pending_writes.front().packet).dump() + '\n';

  spdlog::debug("Calling asio::async_write");
  asio::async_write(
      _socket, asio::buffer(_write_buf),
      [self{shared_from_this()}](std::error_code ec, std::size_t length) {
        spdlog::debug("Writing to socket: {}", self->_write_buf);
        spdlog::debug("Written length: {}", length);

        spdlog::debug("ec: {}", ec.message());
        if (ec == asio::error::eof) {
          return;
        }
        if (ec) {
          spdlog::warn("Error occurred: {}", ec.message());
          return;
        }

        try {
          self->_pending_writes.front().post_write();
          self->_pending_writes.pop();
          if (!self->_pending_writes.empty()) {
            self->do_async_write(); // Try scheduling next unsent packet.
          }
        }
        catch (std::runtime_error &re) {
          spdlog::error("Error occurred: {}, closing this session.", re.what());
        }
      });
}
