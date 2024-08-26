#include "application.h"
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <print>
#include <spdlog/spdlog.h>

void error_callback(int error, const char *description)
{
  spdlog::error("GLFW: {}", description);
}

auto initialize() -> bool
{
  spdlog::set_level(spdlog::level::debug);

  if (!glfwInit()) {
    // Initialization failed
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwSetErrorCallback(error_callback);
  return true;
}

auto main(int argc, char *argv[]) -> int
{
  initialize();

  GLFWwindow *window{glfwCreateWindow(640, 480, "My Title", NULL, NULL)};
  if (!window) {
    throw "Window or OpenGL context creation failed";
  }
  glfwMakeContextCurrent(window);
  gladLoadGLLoader(glfwGetProcAddress);

  try {
    if (argc != 2) {
      std::println("Usage: little-sb-client <player-name>");
      return 0;
    }

    Application app{"localhost", 1438, argv[1]};
    app.run();
  }
  catch (std::exception &e) {
    spdlog::critical("Exception: {}", e.what());
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
