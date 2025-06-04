#pragma once

#include <glm/gtc/type_precision.hpp>

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

// forward declarations

extern GLFWwindow* initialize(glm::uvec2 const& resolution, unsigned ver_major, unsigned ver_minor);

namespace window_handler { 
  // create window and set callbacks
  // load shader programs and update uniform locations

inline  void set_callback_object(GLFWwindow* window, void* app) {
      // set user pointer to access this instance statically
      glfwSetWindowUserPointer(window, app);
  }
inline  void set_button_callback(GLFWwindow* window, void (*func)(GLFWwindow* w, int a, int b, int c)) {
      glfwSetMouseButtonCallback(window, func);
  }

inline void set_scroll_callback(GLFWwindow* window, void (*func)(GLFWwindow*, double, double)) {
      glfwSetScrollCallback(window, func);
  }

inline void set_resize_callback(GLFWwindow* window, void (*func)(GLFWwindow* w, int width, int height)) {
      glfwSetFramebufferSizeCallback(window, func);
  }

inline  void set_key_callback(GLFWwindow* window, void (*func)(GLFWwindow* w, int key, int scancode, int action, int mods)) {
      glfwSetKeyCallback(window, func);
  }

inline  void set_char_callback(GLFWwindow* window, void (*func)(GLFWwindow* w, unsigned int unicode_codepoint)) {
      glfwSetCharCallback(window, func);
  }

inline  void set_char_mods_callback(GLFWwindow* window, void (*func)(GLFWwindow* w, unsigned int unicode_codepoint, int mods)) {
      glfwSetCharModsCallback(window, func);
  }

inline void set_mouse_callback(GLFWwindow* window, void (*func)(GLFWwindow*, double x, double y)) {
      glfwSetCursorPosCallback(window, func);
  }

  // free resources
  void close_and_quit(GLFWwindow* window, int status);
}

