#pragma once

#include "glfw.h"
#include <glad/gl.h>
#include <spdlog/spdlog.h>

void error_callback(int error, const char *description);

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods);

class Window {
  friend void key_callback(GLFWwindow *window, int key, int scancode,
                           int action, int mods);

public:
  static void initialize();
  static void deinitialize();

  Window();
  Window(const Window &) = delete;
  Window(Window &&) = delete;
  auto operator=(const Window &) -> Window & = delete;
  auto operator=(Window &&) -> Window & = delete;
  virtual ~Window();

  virtual void key_callback(int key, int /*scancode*/, int action,
                            int /*mods*/);

  [[nodiscard]] auto should_close() const -> bool;
  void should_close(bool value);

  void use() const;
  void poll_events() const;
  void render() const;

  // Sets the font scale of the window.
  void font_scale(float scale);

  // After calling this, you mustn't call another instance's
  // `Window::pane_begin()`. This is an undefined behavior.
  void pane_begin(std::string const &name) const;

  // Must be called right after this instance's `Window::pane_begin(std::string
  // const &)`.
  void pane_end() const;

  // Creates a button on current pane.
  [[nodiscard]] auto button(std::string const &label) const -> bool;

  // Creates text on current pane.
  void text(std::string const &text) const;

private:
  static std::unordered_map<GLFWwindow *, Window *> forward_table;

  GLFWwindow *_window;
};
