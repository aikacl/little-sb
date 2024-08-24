#pragma once

#include "handle-error.h"
#include "sb-packet.h"
#include "session_fwd.h"
#include <asio.hpp>
#include <queue>

using asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
  static constexpr std::size_t buffer_size{1024};

public:
  Session() = delete;

  Session(tcp::socket socket) : _socket{std::move(socket)} {}

  [[nodiscard]] auto socket() -> tcp::socket &
  {
    return _socket;
  }

  // This also extends its lifetime when mangaged by std::shared_ptr
  void start(std::function<bool(Sb_packet const &)> on_reading_packet,
             std::function<void()> post_session_end)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    _should_stop = false;
    do_async_read(std::move(on_reading_packet), std::move(post_session_end));
  }

  void stop()
  {
    _should_stop = true;
  }

  void write(Sb_packet packet)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    _packets_queue.push(std::move(packet));
    write();
  }

  auto read() -> Sb_packet
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    std::error_code ec;
    std::array<char, buffer_size> buffer{};
    auto const read_length{_socket.read_some(asio::buffer(buffer), ec)};
    handle_error(ec);
    auto const packet{json::parse(std::string_view{buffer.data(), read_length})
                          .get<Sb_packet>()};
    spdlog::debug("Read packet: {}", packet);
    return packet;
  }

  auto request(Sb_packet packet) -> Sb_packet
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    write(std::move(packet));
    return read();
  }

private:
  void do_async_read(std::function<bool(Sb_packet const &)> &&on_reading_packet,
                     std::function<void()> &&post_session_end)
  {
    spdlog::trace("Call {}, Scheduling reading packet",
                  std::source_location::current().function_name());

    _socket.async_read_some(
        asio::buffer(_buffer),
        // Captures self by value to extend session's lifetime
        [self{shared_from_this()},
         on_reading_packet{std::move(on_reading_packet)},
         post_session_end{std::move(post_session_end)}](
            std::error_code const ec, std::size_t const read_length) mutable {
          spdlog::trace("Call {}",
                        std::source_location::current().function_name());

          spdlog::trace("Session use count: {}", self.use_count());
          spdlog::trace("Session address in memory (read packet): {}",
                        static_cast<void const *>(self.get()));
          // spdlog::debug("Session reads new data, ec: {}", ec.message());
          if (ec == asio::error::eof) {
            post_session_end();
            return;
          }
          if (ec) {
            // Do not throw exception because client can construct a malformed
            // package, or just not responding to us. Throwing exception can
            // cause server stop.
            spdlog::error("Error occurred: {}, closing this session.",
                          ec.message());
            post_session_end();
            return;
          }

          auto const packet{
              json::parse(std::string_view{self->_buffer.data(), read_length})
                  .get<Sb_packet>()};
          spdlog::debug("Read packet: {}", packet);

          // Same as above
          try {
            if (on_reading_packet(packet) && !self->_should_stop) {
              self->do_async_read(std::move(on_reading_packet),
                                  std::move(post_session_end));
            }
            else {
              post_session_end();
            }
          }
          catch (std::runtime_error &re) {
            spdlog::error("Error occurred: {}, closing this session.",
                          re.what());
            post_session_end();
            return;
          }
        });
  }
  void write()
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    while (!_packets_queue.empty()) {
      auto const &packet{_packets_queue.front()};
      std::error_code ec;
      asio::write(_socket, asio::buffer(json(packet).dump()), ec);
      spdlog::debug("Sent packet: {}", packet);
      handle_error(ec);
      _packets_queue.pop();
    }
  }

  tcp::socket _socket;
  // Two statement below are for asynchronoized operations
  std::atomic<bool> _should_stop; // For Session::start()
  // In asynchronoized environment, this as may be read in future, should be
  // placed in member field to keep its lifetime
  std::array<char, buffer_size> _buffer{};
  // This is for synchronoized operations
  std::queue<Sb_packet> _packets_queue;
};
