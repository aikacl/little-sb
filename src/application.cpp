#include "application.h"
#include "split-by.h"
#include <iostream>

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
        spdlog::info("Server message: {}", packet.payload);
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
  _subscribing_session->write(Sb_packet{
      Sb_packet_sender{_player_name, _player_name}, json("Subscribe").dump()});

  spdlog::info("Connected to the server: {}", request<std::string>("Request"));

  while (!should_stop()) {
    _io_context.poll();
    tick();
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

void Application::write(std::string message)
{
  _requesting_session->write(
      Sb_packet{Sb_packet_sender{_player_name, _player_name},
                json(std::move(message)).dump()});
}

auto Application::should_stop() const -> bool
{
  return _state == State::Should_stop;
}

void Application::tick()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  switch (_state) {
  case State::Greeting:
    handle_greeting();
    return;
  case State::Starting:
    handle_starting();
    return;
  case State::Running:
    handle_running();
    return;
  case State::Ended:
    handle_ended();
    return;
  default: // Unreachable
    return;
  }
}

void Application::handle_greeting()
{
  spdlog::info("Type \"start\" to begin the game!");
  std::string choice;
  std::cin >> choice;
  std::cin.ignore();
  if (choice == "start") {
    start_new_game(_game_id);
  }
  else {
    spdlog::info("Unknown command '{}'", choice);
  }
}

void Application::handle_starting()
{
  /*std::println("Game started. The following is the health of players:");*/
  /*for (auto const &player : _players) {*/
  /*  std::println("{} has {} health.", player.name(), player.health());*/
  /*}*/
  /*std::println("");*/
  _state = State::Running;
}

void Application::handle_running()
{
  constexpr std::chrono::milliseconds waiting{300};
  std::this_thread::sleep_for(waiting);
  poll_events();
}

void Application::poll_events()
{
  for (;;) {
    auto const event{
        request<std::string>(std::format("query event {}", _game_id))};

    if (event == "no") {
      break;
    }

    auto const parts{split_by(event)};
    spdlog::info("Event: {}", event);

    if (event == "ended") {
      _state = State::Ended;
      break;
    }
  }
}

void Application::handle_ended()
{
  spdlog::info("Continue? y/[n]");
  std::string choice;
  std::getline(std::cin, choice);
  if (choice == "y") {
    start_new_game(_game_id);
  }
  else {
    _state = State::Should_stop;
  }
}

void Application::start_new_game(std::uint64_t &game_id)
{
  std::vector<std::string> parts;

  for (;;) {
    spdlog::info("Players online: {}", request<std::string>("list-players"));
    spdlog::info("Who do you want battle with?");
    std::string choice;
    std::getline(std::cin, choice);
    auto const response{request<std::string>(std::format("battle {}", choice))};
    parts = split_by(response);
    if (parts[0] == "ok") {
      break;
    }
    spdlog::warn("{}", response);
  }

  game_id = std::stoull(parts[1]);
  poll_events();
  _state = State::Starting;
}
