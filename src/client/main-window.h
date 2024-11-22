#pragma once

#include "window.h"

class Main_window : public Window {
public:
  void key_callback(int key, int /*scancode*/, int action, int /*mods*/) final;
};
