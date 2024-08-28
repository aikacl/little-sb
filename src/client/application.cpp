#include "application.h"
#include "event.h"
#include "handle-error.h"
#include "imgui.h"
#include "player.h"
#include <asio.hpp>
#include <cassert>

Application::Application(std::string_view const host, std::uint16_t const port)
    : _session{std::make_shared<Session>(connect(_io_context, host, port))}
{
  spdlog::trace("Call {}", std::source_location::current().function_name());
}

void Application::run()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  while (!should_stop() && !_window.should_close()) {
    // TODO: REMOVE THIS
    // async_request(Command{"login"}, [this](Event const &e) {
    //   if (e.name() != "ok") {
    //     assert(false);
    //   }
    //   _you = std::make_shared<Player>(e.get_arg<Player>(0));
    //   schedule_continuous_query_event();
    //   _state = State::greeting;
    // });
    tick();
  }
}

auto Application::should_stop() const -> bool
{
  return _state == State::should_stop;
}

void Application::tick()
{
  spdlog::trace("Ticking");
  poll_events();
  update();
  render();
}

void Application::poll_events()
{
  _io_context.poll(); // TODO(shelpam): MAYBE WRONG: This should be
                      // called only after _window.poll_events()
  if (_io_context.stopped()) {
    _io_context.restart();
  }
  _window.poll_events();
}

void Application::update()
{
  // Async query event would be started before main loop.

  if (_state == State::starting) {
    async_request(Command{"list-players"}, [this](Event const &e) {
      if (e.name() != "ok") {
        spdlog::warn("list-players returns {}, which is impossible.", e.name());
        return;
      }

      _players = e.get_arg<std::map<std::string, Player>>(0);
      spdlog::debug("Players: {}", json(_players).dump());
    });
  }
}

void Application::render()
{
  if (_you) {
    show_user_info();
  }

  switch (_state) {
  case State::unlogged_in:
    static std::array<char, 32> name_buf{};
    ImGui::InputText("Your name here", name_buf.data(), name_buf.size());
    _name = std::string(name_buf.data(), std::strlen(name_buf.data()));
    if (ImGui::Button("Login")) {
      if (_name.empty()) {
        add_to_show("error: Your name can not be empty. Please retry.");
        return;
      }
      async_request(Command{"login"}, [this](Event const &e) {
        if (e.name() != "ok") {
          assert(false);
        }
        _you = std::make_shared<Player>(e.get_arg<Player>(0));
        schedule_continuous_query_event();
        _state = State::greeting;
      });
      _state = State::logging;
    }
    break;
  case State::logging:
    _window.text("Logging in...");
    break;
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

void Application::show_user_info()
{
  _window.text(std::format("Your name: {}", _you->name()));
  _window.text(std::format("Your health: {}", _you->health()));
  _window.text(std::format("Your damage: {}", _you->damage()));
  _window.text(std::format("Your defense: {}", _you->defense()));
}

void Application::handle_greeting()
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  // ImGui::LabelText("lebel", "fmt");
  static std::array<char, 32> buf{};
  ImGui::InputTextWithHint("Message", "type your message here...", buf.data(),
                           buf.size());
  if (ImGui::Button("send")) {
    Command say{"say"s};
    say.add_arg(std::string{buf.data(), std::strlen(buf.data())});
    async_request(say, [](Event const &e) {
      if (e.name() == "ok") {
        spdlog::info("Message successfully sent.");
      }
      else {
        spdlog::warn("Failed to send message.");
      }
    });
    // Keep state unchanged.
  }
  if (ImGui::Button("start")) {
    _state = State::starting;
  }
  if (ImGui::Button("fuck")) {
    async_request(Command{"fuck"}, [this](Event const &event) {
      if (event.name() == "ok") {
        add_to_show("Fuck succeeded");
      }
    });
  }
}

void Application::handle_starting()
{
  spdlog::trace("Entering {}", std::source_location::current().function_name());

  starting_new_game();
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

void Application::starting_new_game()
{
  _window.text("Making new game...");
  _window.text("Who do you want battle with?");
  if (ImGui::Button("Go back")) {
    _state = State::greeting;
  }
  for (auto const &[_, player] : _players) {
    if (ImGui::Button(player.name().c_str())) {
      Command battle{"battle"};
      battle.add_arg(player.name());
      async_request(battle, [this](Event const &e) {
        if (e.name() == "ok") {
          _game_id = e.get_param<std::size_t>("game-id");
          _state = State::running;
        }
        else if (e.name() == "error") {
          spdlog::warn("{}", e.get_arg<std::string>(0));
          add_to_show("warning: " + e.get_arg<std::string>(0));
        }
        else {
          spdlog::warn("Unknown result: {}", e.name());
        }
      });
    }
  }
}

void Application::process_event(Event const &event)
{
  if (event.name() == "none") {
    return;
  }

  spdlog::info("Event: {}", event.dump());

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
    if (who == _you->name()) {
      _you->take_damage(drop);
    }
  }
  else if (event.name() == "game-end") {
    add_to_show("Game ended.");
    if (_you->health() == 0) {
      add_to_show("You have lost.");
    }
    else {
      add_to_show("You are victorior.");
    }
    _state = State::ended;
  }
  else if (event.name() == "message") {
    if (event.get_arg<std::string>(0) == "game-end") {
      add_to_show("Game ended.");
      if (_you->health() == 0) {
        add_to_show("You have lost.");
      }
      else {
        add_to_show("You are victorior.");
      }
    }
    else {
      add_to_show(event.get_arg<std::string>(0));
    }
  }
  else {
    spdlog::warn("Unknown event: {}", event.name());
  }
}

void Application::add_to_show(std::string message)
{
  constexpr double deferred{10};
  _messages.insert({glfwGetTime() + deferred, std::move(message)});
}

void Application::async_request(Command const &command,
                                std::function<void(Event)> on_replied)
{
  spdlog::debug("Scheduling request: {}", command.dump());
  Packet packet{Packet_sender{_name, _name}, command.dump()};
  _session->schedule_request(
      std::move(packet), [on_replied{std::move(on_replied)}](Packet packet) {
        on_replied(Event{json::parse(std::move(packet.payload))});
      });
}

auto connect(asio::io_context &io_context, std::string_view host,
             std::uint16_t port) -> tcp::socket
{
  spdlog::trace("Call {}", std::source_location::current().function_name());

  tcp::socket socket{io_context};
  auto endpoints{tcp::resolver{io_context}.resolve(host, std::to_string(port))};
  std::error_code ec;
  asio::connect(socket, endpoints, ec);
  handle_error(ec);
  return socket;
}

void Application::schedule_continuous_query_event()
{
  async_request(Command{"query-event"}, [this](Event const &event) {
    process_event(event);
    schedule_continuous_query_event();
  });
}
