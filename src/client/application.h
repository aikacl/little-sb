#pragma once

#include "command.h"
#include "packet.h"
#include "player.h"
#include "session.h"
#include "window.h"

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
              std::string player_name);
  void run();

private:
  [[nodiscard]] auto should_stop() const -> bool;
  void write(Session_ptr const &session, Command const &command);
  void tick();
  void poll_events();
  void update();
  void render();
  void handle_greeting();
  void handle_starting();
  void handle_running();
  void handle_ended();
  void start_new_game();

  void add_to_show(std::string message);

  void check_login();

  template <typename Result_type>
  auto request(Command const &command) -> Result_type
  {
    check_login();

    Packet sent{Packet_sender{_name, _name}, command.dump()};
    Packet read{_session->request(std::move(sent))};
    return json::parse(std::move(read.payload)).get<Result_type>();
  }

  asio::io_context _io_context;
  Session_ptr _session;
  State _state{State::greeting};
  std::string _name;
  Player _you;
  std::uint64_t _game_id{};
  Player _enemy;
  Window _window;
  std::map<double, std::string> _messages;
};

[[nodiscard]] auto connect(asio::io_context &io_context, std::string_view host,
                           std::uint16_t port) -> tcp::socket;
