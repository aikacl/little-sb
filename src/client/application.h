#pragma once

#include "command.h"
#include "packet.h"
#include "session.h"

class Application {
  enum class State : std::uint8_t {
    greeting,
    starting,
    running,
    ended,
    should_stop,
  };

public:
  Application(std::string_view host, std::uint16_t port,
              std::string_view player_name);
  void run();

private:
  [[nodiscard]] auto should_stop() const -> bool;
  void write(Session_ptr const &session, Command const &command);
  void poll_events();
  void tick();
  void handle_greeting();
  void handle_starting();
  void handle_running();
  void handle_ended();
  void start_new_game();

  template <typename Result_type>
  auto request(Command const &command) -> Result_type
  {
    Packet in{Packet_sender{_name, _name}, command.dump()};
    Packet out{_requesting_session->request(std::move(in))};
    return json::parse(std::move(out.payload)).get<Result_type>();
  }

  asio::io_context _io_context;
  Session_ptr _subscribing_session;
  Session_ptr _requesting_session;
  State _state{State::greeting};
  std::string _name;
  std::uint64_t _game_id{};
};

[[nodiscard]] auto connect(asio::io_context &io_context, std::string_view host,
                           std::uint16_t port) -> tcp::socket;
