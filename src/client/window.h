#pragma once

#include "glfw.h"
#include <glad/gl.h>
#include <spdlog/spdlog.h>

void error_callback(int error, const char *description);

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);

class Window {
public:
  static void initialize();
  static void deinitialize();

  Window();
  Window(const Window &) = delete;
  Window(Window &&) = delete;
  auto operator=(const Window &) -> Window & = delete;
  auto operator=(Window &&) -> Window & = delete;
  ~Window();

  [[nodiscard]] auto should_close() const -> bool;

  void use() const;
  void poll_events() const;
  void render() const;

  // After calling this, you mustn't call another instance's
  // `Window::pane_begin()`. This is an undefined behavior.
  void pane_begin(std::string const &name) const;

  // Must be called right after this instance's `Window::pane_begin(std::string
  // const &)`.
  void pane_end() const;

  // Creates a button on current pane.
  auto button(std::string const &label, float scale = 2) const -> bool;

  // Creates text on current pane.
  void text(std::string const &text, float scale = 2) const;

private:
  GLFWwindow *_window;
};
