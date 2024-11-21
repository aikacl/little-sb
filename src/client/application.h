#pragma once

#include "client/message.h"
#include "command.h"
#include "event.h"
#include "game-map.h"
#include "item/item.h"
#include "player.h"
#include "session.h"
#include "window.h"
#include <set>

class Application {
  enum class State : std::uint8_t {
    unlogged_in,
    logging,
    greeting,
    starting_battle,
    battling,
    ended,
    should_stop,
  };

public:
  Application();
  void run();

private:
  // Relative time, since start of the program. Like `glfwGetTime()`.
  [[nodiscard]] auto current_time() const -> Duration;

  [[nodiscard]] auto should_stop() const -> bool;
  void schedule_continuous_query_event();
  void tick();
  void poll_events();
  void update();

  void render();
  void render_unlogged_in();

  void render_messages();

  void show_player_info();
  void handle_greeting();
  void handle_starting_battle();
  void handle_battling();
  void handle_ended();
  void starting_new_game();

  void process_event(Event const &event);
  void add_to_show(std::string message);

  void async_request(Command const &command,
                     std::function<void(Event)> on_replied);

  void remove_expired_messages();
  void connect_to_the_server();

  asio::io_context _io_context;
  Session_ptr _session;

  std::chrono::time_point<std::chrono::steady_clock, std::chrono::nanoseconds>
      _start_time;

  State _state{State::unlogged_in};
  std::string _name;

  Window _window;

  player::Player_ptr _you;

  std::size_t _battle_id{};
  std::size_t _battled_rounds{};

  std::shared_ptr<Game_map> _game_map;

  std::set<Message> _messages;
  std::map<std::string, player::Player> _players;
  std::map<std::string, item::Item_info> _store_items;

  static constexpr std::size_t buf_size{32};
  std::array<char, buf_size> _name_buf{};
  std::array<char, buf_size> _host_buf{"154.7.177.38"};
  static constexpr std::size_t message_max_size{128};
  std::array<char, message_max_size> _message_input_buf{};
};

[[nodiscard]] auto connect(asio::io_context &io_context, std::string_view host,
                           std::string_view port) -> tcp::socket;
