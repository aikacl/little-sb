#include "application.h"
#include "imgui.h"
#include "player.h"
#include "server/session-service.h"
#include <asio.hpp>
#include <map>
#include <utility>

Application::Application(std::string_view const host, std::uint16_t const port,
                         std::string name)
    : _session{std::make_shared<Session>(connect(_io_context, host, port))},
      _name{std::move(name)},
      _you{json::parse(request<std::string>(Command{"query-player"s}))
               .get<Player>()}
{
  spdlog::trace("Call {}", std::source_location::current().function_name());
}

void Application::run()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  while (!should_stop() && !_window.should_close()) {
    tick(); // TODO(shelpam): this shouldn't be blocking.
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

void Application::tick()
{
  spdlog::trace("Entering {}", std::source_location::current().function_name());

  poll_events();
  update();
  render();

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::poll_events()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  _io_context.poll();
  _window.poll_events();

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::update()
{
  using Event = Command;
  while (true) {
    Event event{json::parse(request<std::string>(Command{"query-event"}))};

    if (event.name() == "none") {
      break;
    }

    if (event.name() == "broadcast") {
      auto const from{event.get_param<std::string>("from")};
      auto const what{event.get_arg<std::string>(0)};
      spdlog::info("{} said: {}", from, what);
      add_to_show(std::format("{} said: {}", from, what));
    }
    else if (event.name() == "battle") {
      add_to_show(std::format("{} called for a fight with you.",
                              event.get_param<std::string>("from")));
    }
    else if (event.name() == "fuck") {
      add_to_show(std::format("You are fucked by {}",
                              event.get_param<std::string>("fucker")));
    }
    else if (event.name() == "health-drop") {
      auto const who{event.get_param<std::string>("player")};
      auto const drop{event.get_param<int>("drop")};
      add_to_show(std::format("{} has dropped {} health", who, drop));
      if (who == _you.name()) {
        _you.take_damage(drop);
      }
    }
    else if (event.name() == "game-end") {
      add_to_show("Game ended.");
      if (_you.health() == 0) {
        add_to_show("You have lost.");
      }
      else {
        add_to_show("You are victorior.");
      }
      _state = State::ended;
    }
    else {
      spdlog::warn("Unknown event: {}", event.name());
    }
  }
}

void Application::render()
{
  _window.text(std::format("Your name: {}", _you.name()));
  _window.text(std::format("Your health: {}", _you.health()));
  _window.text(std::format("Your damage: {}", _you.damage()));
  _window.text(std::format("Your defense: {}", _you.defense()));

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

  auto const now{glfwGetTime()};
  for (auto it{_messages.begin()}; it != _messages.end();) {
    if (it->first < now) {
      it = _messages.erase(it);
    }
    else {
      _window.text(it->second);
      ++it;
    }
  }

  _window.render();
}

void Application::handle_greeting()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  // ImGui::LabelText("lebel", "fmt");
  static std::string buf(32, '\0');
  ImGui::InputTextWithHint("Message", "type your message here...", buf.data(),
                           buf.size());
  if (ImGui::Button("send")) {
    Command say{"say"s};
    say.add_arg(buf);
    assert(json::parse(request<std::string>(say)).get<std::string>() == "ok");
    // Keep state unchanged.
  }
  if (ImGui::Button("start")) {
    _state = State::starting;
  }
  if (ImGui::Button("fuck")) {
    Command fuck{"fuck"s};
    auto response(json::parse(request<std::string>(fuck)));
    if (response.get<std::string>() == "ok") {
      add_to_show("Fuck succeeded");
    }
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

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::handle_running()
{
  spdlog::trace("Entering {}", std::source_location::current().function_name());

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::handle_ended()
{
  spdlog::trace("Entering {}", std::source_location::current().function_name());

  _window.text("Continue?");
  if (ImGui::Button("Yes")) {
    _state = State::starting;
  }
  if (ImGui::Button("No")) {
    _state = State::greeting;
  }

  spdlog::trace("Leaving {}", std::source_location::current().function_name());
}

void Application::start_new_game()
{
  auto listed_players{request<std::string>(Command{"list-players"s})};
  auto const players{
      json::parse(listed_players).get<std::map<std::string, Player>>()};
  spdlog::debug("Players online: {}", listed_players);
  _window.text("Who do you want battle with?");

  for (auto const &[_, player] : players) {
    if (ImGui::Button(player.name().c_str())) {
      Command battle{"battle"s};
      battle.add_arg(player.name());
      Command result{json::parse(request<std::string>(battle))};
      if (result.name() == "ok") {
        _game_id = result.get_param<std::size_t>("game-id");
        _state = State::running;
      }
      else if (result.name() == "error") {
        spdlog::warn("{}", result.get_arg<std::string>(0));
      }
      else {
        spdlog::warn("Unknow result: {}", result.name());
      }
    }
  }
}

void Application::add_to_show(std::string message)
{
  constexpr double deferred{10};
  _messages.insert({glfwGetTime() + deferred, std::move(message)});
}

void Application::check_login()
{
  static bool logged_in{};
  if (logged_in) {
    return;
  }
  logged_in = true;
  Command login{"login"s};
  spdlog::info("Connected to the server: {}", request<std::string>(login));
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
