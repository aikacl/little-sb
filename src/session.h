#pragma once

#include "packet.h"
#include "session_fwd.h"
#include <asio.hpp>
#include <queue>

using asio::ip::tcp;

// There will be a worker keeping to check pending packets and to send them.
// But if no pending packet exists, the worker stops working. And now there be a
// reading worker same as sending worker.
class Session : public std::enable_shared_from_this<Session> {
  struct Write {
    Packet packet;
    std::function<void()> post_write;
  };

public:
  Session() = delete;
  Session(tcp::socket socket);

  void schedule_request(Packet packet, std::function<void(Packet)> on_replied);

  // When read, don't reply
  void schedule_read(std::function<void(Packet)> on_read);

  // We guarantee there will be at most one instance of this function running.
  void schedule_write(Packet packet, std::function<void()> post_write);

private:
  void do_async_read();

  void do_async_write();

  tcp::socket _socket;
  std::queue<std::function<void(Packet)>> _pending_reads;
  std::queue<Write> _pending_writes;
  // asio::streambuf _streambuf;
  std::string _read_buf;
  std::string _write_buf;
};
