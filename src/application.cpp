#include "application.h"
#include "command.h"
#include "sb-packet.h"
#include "session.h"
#include "split-by.h"
#include <asio.hpp>
#include <cassert>
#include <iostream>
#include <source_location>
#include <spdlog/spdlog.h>
#include <string>

Application::Application(std::string_view const host, std::uint16_t const port,
                         std::string_view const player_name)
    : _subscribing_session{std::make_shared<Session>(connect(host, port))},
      _requesting_session{std::make_shared<Session>(connect(host, port))},
      _player_name{player_name}
{
  spdlog::trace("Call {}", std::source_location::current().function_name());
}

void Application::run()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  _subscribing_session->start(
      [](Sb_packet const &packet) {
        Command const command{json::parse(packet.payload)};
        if (command.name() == "broadcast") {
          auto const from{command.get_param<std::string>("from")};
          auto const what{command.get_arg<std::string>(0)};
          spdlog::info("{} said: {}", from, what);
        }
        if (command.name() == "event") {
          spdlog::info("Event received: {}", command.get_arg<std::string>(0));
        }
        return true;
      },
      []() {});

  // We should first write subscribing session and then requesting session,
  // because the server would expect for subscribing session to connect first.
  // If not, saying that requesting session post request first, the server
  // would wait for subscribing session to connect to it, which is a waste of
  // server resources.
  // We shouldn't use `request` here because we previously called
  // `Session::start`, which will catch packet for use, conflicting with read
  // packet here.
  Command subsribe{"Subscribe"s};
  write(_subscribing_session, subsribe);

  Command request1{"Request"s};
  spdlog::info("Connected to the server: {}", request<std::string>(request1));

  while (!should_stop()) {
    _io_context.poll();
    tick(); // TODO(shelpam): This shouldn't be blocking.
  }

  _subscribing_session->stop();
  _requesting_session->stop();
}

auto Application::connect(std::string_view const host,
                          std::uint16_t const port) -> tcp::socket
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  tcp::socket socket{_io_context};
  auto const endpoints{
      tcp::resolver{_io_context}.resolve(host, std::to_string(port))};
  std::error_code ec;
  asio::connect(socket, endpoints, ec);
  handle_error(ec);
  return socket;
}

void Application::write(Session_ptr const &session, Command const &command)
{
  session->write(
      Sb_packet{Sb_packet_sender{_player_name, _player_name}, command.dump()});
}

auto Application::should_stop() const -> bool
{
  return _state == State::should_stop;
}

void Application::tick()
{
  spdlog::trace("Entering {}", std::source_location::current().function_name());

  switch (_state) {
  case State::greeting:
    handle_greeting();
    break;
  case State::starting:
    handle_starting();
    break;
  case State::running:
    handle_running();
    break;
  case State::ended:
    handle_ended();
    break;
  default: // Unreachable
    break;
  }

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::handle_greeting()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  spdlog::info("Type \"start\" to begin the game!");
  spdlog::info("Use \"say <sentence>\" to speak to everyone!");
  std::string choice;
  std::getline(std::cin, choice);
  if (choice == "start") {
    _state = State::starting;
  }
  else if (choice.substr(0, 4) == "say ") {
    Command say{"say"s};
    say.add_arg(choice.substr(4));
    if (request<std::string>(say) != "ok") {
      assert(false); // TODO(shelpam): Improve this; seems possible but when
                     // will it present?
    };
    // We keep state unchanged.
  }
  else {
    spdlog::info("Unknown command '{}'", choice);
  }
}

void Application::handle_starting()
{
  spdlog::trace("Entering {}", std::source_location::current().function_name());

  start_new_game();
  /*std::println("Game started. The following is the health of players:");*/
  /*for (auto const &player : _players) {*/
  /*  std::println("{} has {} health.", player.name(), player.health());*/
  /*}*/
  /*std::println("");*/
  _state = State::running;

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::handle_running()
{
  spdlog::trace("Entering {}", std::source_location::current().function_name());

  constexpr std::chrono::milliseconds waiting{300};
  std::this_thread::sleep_for(waiting);
  poll_events();

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::poll_events()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  while (true) {
    Command query_event{"query-event"s};
    query_event.add_arg(_game_id);
    auto const event{request<std::string>(query_event)};

    if (event == "no") {
      break;
    }

    spdlog::info("Event: {}", event);

    if (event == "ended") {
      _state = State::ended;
      break;
    }
  }

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::handle_ended()
{
  spdlog::trace("Entering {}", std::source_location::current().function_name());

  spdlog::info("Continue? y/[n]");
  std::string choice;
  std::getline(std::cin, choice);
  if (choice == "y") {
    _state = State::starting;
  }
  else {
    _state = State::greeting;
  }

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::start_new_game()
{
  std::vector<std::string> parts;

  while (true) {
    spdlog::info("Players online: {}",
                 request<std::string>(Command{"list-players"s}));
    spdlog::info("Who do you want battle with?");
    std::string choice;
    std::getline(std::cin, choice);
    Command battle{"battle"s};
    battle.add_arg(choice);
    auto const response{request<std::string>(battle)};
    parts = split_by(response);
    if (parts[0] == "ok") {
      break;
    }
    spdlog::warn("{}", response);
  }

  _game_id = std::stoull(parts[1]);
}
