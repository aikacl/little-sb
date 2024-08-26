#include "application.h"
#include "split-by.h"
#include <asio.hpp>
#include <iostream>

Application::Application(std::string_view const host, std::uint16_t const port,
                         std::string player_name)
    : _session{std::make_shared<Session>(connect(_io_context, host, port))},
      _name{std::move(player_name)}
{
  spdlog::trace("Call {}", std::source_location::current().function_name());
}

void Application::run()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  Command login{"login"s};
  spdlog::info("Connected to the server: {}", request<std::string>(login));

  while (!should_stop() && !_window.should_close()) {
    _io_context.poll();
    // tick(); // TODO(shelpam): this shouldn't be blocking.
    _window.poll_events();
  }

  _session->stop();
}

void Application::write(Session_ptr const &session, Command const &command)
{
  session->write(Packet{Packet_sender{_name, _name}, command.dump()});
}

auto Application::should_stop() const -> bool
{
  return _state == State::should_stop;
}

void Application::poll_events()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  while (true) {
    Command cmd{"event"s};
    auto const event{request<std::string>(cmd)};

    if (event == "") {
      break;
    }

    Command const command{json::parse(event)};
    if (command.name() == "broadcast") {
      auto const from{command.get_param<std::string>("from")};
      auto const what{command.get_arg<std::string>(0)};
      spdlog::info("{} said: {}", from, what);
    }
    else if (command.name() == "event") {
      spdlog::info("Event received: {}", command.get_arg<std::string>(0));
    }
    else if (command.name() == "fuck") {
      spdlog::info("Event received: You are fucked by {}",
                   command.get_param<std::string>("fucker"));
    }
    else {
      spdlog::warn("Unknown event: {}", event);
    }
  }

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::tick()
{
  spdlog::trace("Entering {}", std::source_location::current().function_name());
  constexpr std::chrono::milliseconds waiting{300};
  std::this_thread::sleep_for(waiting);
  poll_events();

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
    if (json::parse(request<std::string>(say)).get<std::string>() !=
        "ok, from server commands") {
      assert(false); // TODO(shelpam): Improve this; seems possible but when
                     // will it present?
    }
    // Keep state unchanged.
  }
  else if (choice == "fuck") {
    Command fuck{"fuck"s};
    auto response(json::parse(request<std::string>(fuck)));
    if (response.get<std::string>() == "ok") {
      spdlog::info("Fuck succeeded");
    }
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

  while (true) {
    Command query_event{"query-event"s};
    query_event.add_arg(_game_id);
    auto event_json_str{request<std::string>(query_event)};
    spdlog::debug("Requested json: {}", event_json_str);
    auto const event{json::parse(std::move(event_json_str)).get<std::string>()};

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
  _state = choice == "y" ? State::starting : State::greeting;

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::start_new_game()
{
  while (true) {
    auto const players_list{request<std::string>(Command{"list-players"s})};
    spdlog::info("Players online: {}", players_list);
    spdlog::info("Who do you want battle with?");
    std::string choice;
    std::getline(std::cin, choice);
    Command battle{"battle"s};
    battle.add_arg(choice);
    auto const response{request<std::string>(battle)};
    auto const parts{split_by(response)};
    if (parts[0] == "ok") {
      _game_id = std::stoull(parts[1]);
      break;
    }
    spdlog::warn("{}", response);
  }
}

auto connect(asio::io_context &io_context, std::string_view const host,
             std::uint16_t const port) -> tcp::socket
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  tcp::socket socket{io_context};
  auto const endpoints{
      tcp::resolver{io_context}.resolve(host, std::to_string(port))};
  std::error_code ec;
  asio::connect(socket, endpoints, ec);
  handle_error(ec);
  return socket;
}
