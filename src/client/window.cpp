#include "window.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void error_callback(int error, const char *description)
{
  spdlog::error("GLFW({}): {}", error, description);
}
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

void Window::initialize()
{
  if (glfwInit() == 0) {
    // Initialization failed
    throw "GLFW initialization failed";
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glfwSetErrorCallback(error_callback);
}

void Window::deinitialize()
{
  glfwTerminate();
}

Window::Window()
    : _window{glfwCreateWindow(640, 480, "Little sb", nullptr, nullptr)}
{
  if (_window == nullptr) {
    throw "Window or OpenGL context creation failed";
  }
  glfwSetKeyCallback(_window, key_callback);
  int width;
  int height;
  glfwGetFramebufferSize(_window, &width, &height);

  use();
  glfwSwapInterval(1);
  glViewport(0, 0, width, height);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(
      _window, true); // Second param install_callback=true will install
                      // GLFW callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();
}

Window::~Window()
{
  glfwDestroyWindow(_window);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void Window::use() const
{
  glfwMakeContextCurrent(_window);
}

void Window::poll_events() const
{
  use();
  glfwPollEvents();
  glClear(GL_COLOR_BUFFER_BIT);

  // (Your code calls glfwPollEvents())
  // ...
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  // ImGui::ShowDemoWindow(); // Show demo window! :)
}

auto Window::button(std::string const &label) -> bool
{
  use();
  return ImGui::Button(label.c_str());
}

void Window::text(std::string const &text)
{
  use();
  ImGui::Text("%s", text.c_str());
}

void Window::render() const
{
  use();
  // Rendering
  // (Your code clears your framebuffer, renders your other stuff etc.)
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  // (Your code calls glfwSwapBuffers() etc.)
  glfwSwapBuffers(_window);
}

auto Window::should_close() const -> bool
{
  return glfwWindowShouldClose(_window) != 0;
}
