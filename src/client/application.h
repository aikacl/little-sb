#pragma once

#include "command.h"
#include "event.h"
#include "item.h"
#include "player.h"
#include "session.h"
#include "window.h"

class Application {
  enum class State : std::uint8_t {
    unlogged_in,
    logging,
    greeting,
    starting,
    running,
    ended,
    should_stop,
  };

public:
  Application(std::string_view host, std::uint16_t port);
  void run();

private:
  [[nodiscard]] auto should_stop() const -> bool;
  void schedule_continuous_query_event();
  void tick();
  void poll_events();
  void update();
  void render();
  void show_user_info();
  void handle_greeting();
  void handle_starting();
  void handle_running();
  void handle_ended();
  void starting_new_game();

  void process_event(Event const &event);
  void add_to_show(std::string message);

  void async_request(Command const &command,
                     std::function<void(Event)> on_replied);

  asio::io_context _io_context;
  Session_ptr _session;
  State _state{State::unlogged_in};
  std::string _name;
  Player_ptr _you;
  std::uint64_t _game_id{};
  std::map<std::string, Player> _players;
  std::map<std::string, Item> _store_items;
  Window _window;
  std::map<double, std::string> _messages;
};

[[nodiscard]] auto connect(asio::io_context &io_context, std::string_view host,
                           std::uint16_t port) -> tcp::socket;
