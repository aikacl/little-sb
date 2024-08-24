#include "server-command-executor.h"
#include "command.h"
#include "game.h"
#include "server.h"

Server_command_executor::~Server_command_executor() = default;

Server_command_executor::Server_command_executor(Server *server)
    : _server{server}
{
}

auto Server_command_executor::server() -> Server *
{
  return _server;
}

auto Say_server_command_executor::execute(std::string from,
                                          Command const &command) -> json
{
  auto const content{command.get_arg<std::string>(0)};
  for (auto const &[name, _] : server()->_publishing_name_to_session) {
    Command broadcast{"broadcast"s};
    broadcast.add_arg(content);
    broadcast.set_param("from", from);
    server()->publish(name, broadcast);
  }
  return "ok, from server commands";
}
Say_server_command_executor::Say_server_command_executor(Server *server)
    : Server_command_executor{server}
{
}
constexpr auto Say_server_command_executor::name() -> std::string
{
  return "say";
}

auto Query_event_server_command_executor::execute(
    std::string /*from*/, Command const &command) -> json
{
  auto const game_id{command.get_arg<std::int64_t>(0)};
  auto &game{server()->_games.at(game_id)};
  if (game.ended()) {
    server()->_games.extract(game_id);
    return "ended";
  }
  auto &events_queue{game.pending_events()};
  if (events_queue.empty()) {
    return "no";
  }
  auto event{std::move(events_queue.front())};
  events_queue.pop();
  return event;
}
Query_event_server_command_executor::Query_event_server_command_executor(
    Server *server)
    : Server_command_executor{server}
{
}
constexpr auto Query_event_server_command_executor::name() -> std::string
{
  return "query-event";
}
