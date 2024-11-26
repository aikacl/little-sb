#pragma once

#include "window.h"

class Application;

class Main_window : public Window {
public:
  Main_window(Application *app);
  void key_callback(int key, int /*scancode*/, int action, int /*mods*/) final;

private:
  Application *_app;
};
