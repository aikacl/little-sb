#pragma once

#include "handle-error.h"
#include "sb-packet.h"
#include <asio.hpp>
#include <functional>
#include <memory>
#include <print>
#include <queue>
#include <source_location>
#include <spdlog/spdlog.h>

using asio::ip::tcp;

class Session;
using Session_ptr = std::shared_ptr<Session>;

class Session : public std::enable_shared_from_this<Session> {
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

  void push(Sb_packet const &packet)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    _packets_queue.push(packet);
  }

  void write()
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    while (!_packets_queue.empty()) {
      auto const &data{_packets_queue.front()};
      std::error_code ec;
      asio::write(_socket, asio::buffer(data), ec);
      spdlog::debug("Sent packet: {}", data);
      handle_error(ec);
      _packets_queue.pop();
    }
  }

  void write(Sb_packet const &packet)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    push(packet);
    write();
  }

  auto read() -> Sb_packet
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    std::error_code ec;
    auto const packet{read(ec)};
    handle_error(ec);
    return packet;
  }

  auto read(std::error_code &ec) -> Sb_packet
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    Sb_packet packet;
    _socket.read_some(asio::mutable_buffer{packet}, ec);
    return packet;
  }

  template <typename Result_type>
  auto request(Sb_packet const &packet) -> Result_type
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());

    push(packet);
    write();
    auto const reply{read()};
    if constexpr (std::same_as<Result_type, std::string>) {
      return reply.to_string();
    }
    else {
      return *static_cast<Result_type const *>(&reply);
    }
  }

private:
  void do_async_read(std::function<bool(Sb_packet const &)> &&on_reading_packet,
                     std::function<void()> &&post_session_end)
  {
    spdlog::trace("Call {}", std::source_location::current().function_name());
    spdlog::trace("Scheduling reading packet");

    _socket.async_read_some(
        asio::mutable_buffer{_packet},
        // Captures self by value to extend session's lifetime
        [self{shared_from_this()},
         on_reading_packet{std::move(on_reading_packet)},
         post_session_end{std::move(post_session_end)}](
            std::error_code const ec, std::size_t const /*length*/) mutable {
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
          spdlog::debug("Read packet: {}", self->_packet);

          // Same as above
          try {
            if (on_reading_packet(self->_packet) && !self->_should_stop) {
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

  std::atomic<bool> _should_stop; // For start()
  tcp::socket _socket;
  Sb_packet _packet; // In asynchronoized environment, this as may be read in
                     // future, should be placed in member field to keep its
                     // lifetime
  std::queue<Sb_packet> _packets_queue;
};
