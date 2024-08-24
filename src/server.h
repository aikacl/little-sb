#pragma once

#include "command.h"
#include "game.h"
#include "sb-packet.h"
#include "session.h"
#include <asio.hpp>
#include <atomic>
#include <map>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

class Server {
  friend class Say_server_command_executor;
  friend class Query_event_server_command_executor;

public:
  void run();
  void shutdown();
  static auto instance(std::uint16_t bind_port = 0) -> Server &;

private:
  explicit Server(std::uint16_t bind_port);
  void
  register_command_executor(std::unique_ptr<Server_command_executor> executor);
  [[nodiscard]] static constexpr auto tick_interval();
  void do_accept();
  // Only accepts publishing sessions
  void close_session_pairs(Session_ptr const &publishing_session);
  void remove_player(std::string const &player_name);
  auto
  wait_for_publishing_session(std::string const &session_name) -> Session_ptr;
  // @return
  //  should continue to read another packet
  auto handle_packet(Session_ptr const &session,
                     Sb_packet const &packet) -> bool;
  auto handle_player_command(Session_ptr const &session,
                             std::string const &from,
                             Command const &command) -> bool;
  auto parse_player_message(std::string const &player_name,
                            Command const &command) -> std::string;
  void publish(std::string const &to, Command const &command);
  void respond(Session_ptr const &session, std::string_view to,
               std::string message);
  auto allocate_game(std::array<Player *, 2> const &players) -> Game &;
  void run_main_game_loop();

  std::atomic<bool> _main_game_loop_should_stop;
  std::map<std::uint64_t, Game> _games;
  std::map<std::string, Player> _players;
  std::map<std::string, Session_ptr> _publishing_name_to_session;
  std::map<Session_ptr, std::string> _publishing_session_to_name;
  std::map<Session_ptr, Session_ptr> _responding_to_publishing;
  asio::io_context _io_context;
  asio::ip::tcp::acceptor _acceptor;
  std::error_code _ec;
  std::string _name{"Server"};
  std::map<std::string, std::unique_ptr<Server_command_executor>>
      _server_commands;
  static constexpr std::size_t max_tick_per_second{60};
};
