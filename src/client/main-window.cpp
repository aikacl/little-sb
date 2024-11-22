#include "main-window.h"

void Main_window::key_callback(int key, int /*scancode*/, int action,
                               int /*mods*/)
{
  // Default: Press Esc to close the window.
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    should_close(true);
  }
}
