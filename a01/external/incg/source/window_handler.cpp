#include "window_handler.hpp"

#include <glbinding/gl/gl.h>
// load glbinding extensions
#include <glbinding/Binding.h>
// load meta info extension
#include <glbinding/Meta.h>
#include <glbinding/Version.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/type_precision.hpp>


#include <iostream>
// helper functions
static void glsl_error(int error, const char* description);
static void watch_gl_errors(bool activate = true);
static void APIENTRY openglCallbackFunction(
  GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam
);



GLFWwindow* initialize(glm::uvec2 const& resolution, unsigned ver_major, unsigned ver_minor) {

  glfwSetErrorCallback(glsl_error);

  if (!glfwInit()) {
    std::exit(EXIT_FAILURE);
  }

  // set OGL version explicitly 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, ver_major);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, ver_minor);
  // enable deug support
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

  //MacOS requires forward compat core profile
  #ifdef __APPLE__
  if (ver_major > 2) {
      // required to prdouce core context on MacOS
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
      // disable deprecated functionality
      if (ver_minor > 1) {
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      }
  }
  #endif

  // create m_window, if unsuccessfull, quit
  GLFWwindow* window = glfwCreateWindow(resolution.x, resolution.y, "OpenGL Framework", NULL, NULL);
  if (!window) {
    glfwTerminate();
    std::exit(EXIT_FAILURE);
  }

  // use the windows context
  glfwMakeContextCurrent(window);
  // disable vsync
  glfwSwapInterval(0);
  // initialize glindings in this context
  glbinding::Binding::initialize();
  // activate error checking after each gl function call
  watch_gl_errors();

  // Enable the debug callback
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(openglCallbackFunction, nullptr);
  glDebugMessageControl(
    GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true
  );

  return window;
}
namespace window_handler {
void close_and_quit(GLFWwindow* window, int status) {

  // free glfw resources
  glfwDestroyWindow(window);
  glfwTerminate();

  std::exit(status);
}

}

///////////////////////////// local helper functions //////////////////////////
static void glsl_error(int error, const char* description) {
  std::cerr << "GLSL Error " << error << " : "<< description << std::endl;
}

static void APIENTRY openglCallbackFunction(
  GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam
){
  (void)source; (void)type; (void)id; 
  (void)severity; (void)length; (void)userParam;
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || type == GL_DEBUG_TYPE_PERFORMANCE) {
    return;
  }
  std::cerr << glbinding::Meta::getString(severity) << " - " << glbinding::Meta::getString(type) << ": ";
  std::cerr << message << std::endl;
  // if (severity != GL_DEBUG_SEVERITY_NOTIFICATION && type != GL_DEBUG_TYPE_PERFORMANCE) {
  //   throw std::runtime_error{"OpenGL error"};
  // }
}

static void watch_gl_errors(bool activate) {
  if(activate) {
    // add callback after each function call
    glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After | glbinding::CallbackMask::ParametersAndReturnValue, {"glGetError", "glBegin", "glVertex3f", "glColor3f"});
    glbinding::setAfterCallback(
      [](glbinding::FunctionCall const& call) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
          // print name
          std::cerr <<  "OpenGL Error: " << call.function->name() << "(";
          // parameters
          for (unsigned i = 0; i < call.parameters.size(); ++i)
          {
            std::cerr << call.parameters[i]->asString();
            if (i < call.parameters.size() - 1)
              std::cerr << ", ";
          }
          std::cerr << ")";
          // return value
          if(call.returnValue) {
            std::cerr << " -> " << call.returnValue->asString();
          }
          // error
          std::cerr  << " - " << glbinding::Meta::getString(error) << std::endl;
          // throw exception to allow for backtrace
          throw std::runtime_error("OpenGl error: " + std::string(call.function->name()));
          exit(EXIT_FAILURE);
        }
      }
    );
  }
  else {
    glbinding::setCallbackMask(glbinding::CallbackMask::None);
  }
}
