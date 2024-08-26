#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <spdlog/spdlog.h>

inline void error_callback(int error, const char *description)
{
  spdlog::error("GLFW: {}", description);
}

inline void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

class Window {
public:
  static void initialize()
  {
    if (!glfwInit()) {
      // Initialization failed
      throw "GLFW initialization failed";
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(error_callback);
  }

  static void deinitialize()
  {
    glfwTerminate();
  }

  Window() : _window{glfwCreateWindow(640, 480, "My Title", NULL, NULL)}
  {
    if (!_window) {
      throw "Window or OpenGL context creation failed";
    }
    glfwSetKeyCallback(_window, key_callback);
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);

    glfwMakeContextCurrent(_window);
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

  ~Window()
  {
    glfwDestroyWindow(_window);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  void poll_events()
  {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);

    // (Your code calls glfwPollEvents())
    // ...
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow(); // Show demo window! :)

    // Rendering
    // (Your code clears your framebuffer, renders your other stuff etc.)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // (Your code calls glfwSwapBuffers() etc.)

    glfwSwapBuffers(_window);
  }

  [[nodiscard]] auto should_close() const -> bool
  {
    return glfwWindowShouldClose(_window);
  }

private:
  GLFWwindow *_window;
};
