#include "server/server.h"
#include "game.h"
#include "player.h"
#include "server/server-command-executor.h"
#include <source_location>
#include <spdlog/spdlog.h>

auto Server::instance(std::uint16_t const bind_port) -> Server &
{
  static Server the_instance{bind_port};
  return the_instance;
}

void Server::run()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  spdlog::info("Server started. Accepting connections...");
  _session_service.start();
  run_main_game_loop();
}

void Server::shutdown()
{
  spdlog::info("Server shutting down...");

  _session_service.stop();
  _main_game_loop_should_stop = true;
}

auto Server::io_context() -> asio::io_context &
{
  return _io_context;
}

Server::Server(std::uint16_t const bind_port)
    : _session_service(this, bind_port, "Server")
{
  register_command_executor(
      std::make_unique<Say_server_command_executor>(this));
  register_command_executor(
      std::make_unique<Query_event_server_command_executor>(this));
  register_command_executor(
      std::make_unique<Fuck_server_command_executor>(this));
}

void Server::register_command_executor(
    std::unique_ptr<Server_command_executor> executor)
{
  _server_commands.insert({executor->name(), std::move(executor)});
}

constexpr auto Server::tick_interval()
{
  return std::chrono::milliseconds{std::chrono::seconds{1}} /
         max_tick_per_second;
}

void Server::remove_player(std::string const &player_name)
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  _players.extract(player_name);
}

auto Server::allocate_game(std::array<Player *, 2> players) -> Game &
{
  auto const id{_games.empty() ? std::uint64_t{} : _games.rbegin()->first + 1};
  return _games.insert({id, Game{id, players, &_session_service}})
      .first->second;
}

void Server::run_main_game_loop()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  auto time_since_last_update{std::chrono::steady_clock::now()};
  while (!_main_game_loop_should_stop) {
    _io_context.poll();

    std::this_thread::sleep_until(time_since_last_update + tick_interval());
    for (auto &[id, game] : _games) {
      game.tick();
    }
    time_since_last_update = std::chrono::steady_clock::now();
  }

  spdlog::trace("Main game loop over");
}

auto Server::verify_userinfo(Packet_sender const &user) const -> bool
{
  // TODO(ShelpAm): replace this placeholder implementation.
  return user.username() == user.password();
}
