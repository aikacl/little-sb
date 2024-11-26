#include "main-window.h"
#include "client/application.h"

void Main_window::key_callback(int key, int /*scancode*/, int action,
                               int /*mods*/)
{
  // Default: Press Esc to close the window.
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    should_close(true);
  }

  if (_app->state() != Application::State::unlogged_in &&
      _app->state() != Application::State::logging) {
    switch (key) {
    case GLFW_KEY_W:
    case GLFW_KEY_A:
    case GLFW_KEY_S:
    case GLFW_KEY_D: {
      switch (action) {
      case GLFW_PRESS:
      case GLFW_REPEAT: {
        static std::unordered_map<int, Vec2> const dirs{
            {GLFW_KEY_W, Vec2{-1, 0}},
            {GLFW_KEY_A, Vec2{0, -1}},
            {GLFW_KEY_S, Vec2{1, 0}},
            {GLFW_KEY_D, Vec2{0, 1}}};
        Command move{"move"};
        move.set_param("direction", dirs.at(key));
        _app->async_request(move, [](Event const &e) {
          if (e.name() != "ok") {
            throw std::runtime_error{e.get_arg<std::string>(0)};
          }
        });
        break;
      }
      case GLFW_RELEASE: {
        Command move{"move"};
        move.set_param("direction", Vec2{});
        _app->async_request(move, [](Event const &e) {
          if (e.name() != "ok") {
            throw std::runtime_error{e.get_arg<std::string>(0)};
          }
        });
        break;
      }
      default:
        break;
      }
      break;
    }
    default:
      break;
    }
  }
}

Main_window::Main_window(Application *app) : _app{app} {}
