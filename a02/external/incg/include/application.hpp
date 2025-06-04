#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <glbinding/gl/types.h>
#include <map>

#include "shader_loader.hpp"
#include <glm/gtc/type_precision.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_orient.h>
// use gl definitions from glbinding
using namespace gl;

#include "helper.hpp"

const int WIDTH  = 1024;
const int HEIGHT = 768;

struct GLFWwindow;

struct CTwBar;
typedef struct CTwBar TwBar;

class Application
{
   public:
    // template<typename T>
    // static void run(int argc, char* argv[], unsigned ver_major = 3, unsigned ver_minor = 2);

    // allocate and initialize objects
    Application(std::string const& resource_path);
    Application(Application const&) = delete;
    Application& operator=(Application const&) = delete;
    // free resources
    virtual ~Application();

    // draw all objects
    virtual void render() = 0;

    virtual void imgui() = 0;

    virtual void update(float dt) = 0;


    void run(int max_fps = 60);

    void imGui_plotFPS();

    void updateCamera();
    // handle key input
    void keyCallback(GLFWwindow* window, int key, int action);
    void charCallback(unsigned int unicode);
    void charModsCallback(unsigned int unicode, int mods);
    // handle mouse movement input
    void cursorCallback(double pos_x, double pos_y);
    // handle mpuse button input
    void buttonCallback(int button, int action, int mods);
    // handle scrolling input
    void scrollCallback(double offset_x, double offset_y);
    // handle resizing
    void resizeCallback(int width, int height);

    void updateShaderPrograms();

   protected:
    // resize callback for derived applications
    virtual void resize();

    uint32_t shader(std::string const& name) const;
    void initializeShader(std::string const& name, std::map<GLenum, std::string> const& files, bool binary = false);

    // get uniform location, throwing exception if name describes no active uniform variable
    GLint glGetUniformLocation(GLuint, const GLchar*) const;

    // upload uniform by name wrapper function
    template <typename T>
    void uniform(std::string const& program, const std::string& name, T const& value) const;
    // uniform upload function
    template <typename T>
    void uniform(uint32_t program, const std::string& name, T const& value) const;

    glm::fmat4 const& viewMatrix() const;
    glm::fmat4 const& projectionMatrix() const;
    glm::uvec2 const& resolution() const;
    std::string m_resource_path;

    // camera
    cameraSystem m_cam;
    float last_frame_time = 0;

   private:
    // shader storage
    std::map<std::string, uint32_t> m_shader_handles{};
    std::map<std::string, std::map<GLenum, std::string>> m_shader_files{};
    // mouse buttons
    bool m_pressed_right;
    bool m_pressed_middle;
    bool m_pressed_left;
    // camera movement factors
    float m_speed_keyboard, m_speed_mouse;
    // transform matrices
    glm::fmat4 m_viewMatrix;
    glm::fmat4 m_projMatrix;
    glm::uvec2 m_resolution;
    GLFWwindow* window;

    int last_frame_times_i = 0;
    int last_frame_times_N = 120;
    float last_frame_times[240];
    float last_frame_fps[240];
    int show_ms_fps = 1; //default fps
};


#include "uniform_upload.hpp"
template <typename T>
void Application::uniform(std::string const& program, const std::string& name, T const& value) const
{
    int handle = m_shader_handles.at(program);
    uniform(handle, name, value);
}

template <typename T>
void Application::uniform(uint32_t program, const std::string& name, T const& value) const
{
    int loc = glGetUniformLocation(program, name.c_str());
    glUniform(loc, value);
}

#include "window_handler.hpp"

// dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

inline std::string read_resource_path(int argc, char* argv[])
{
    std::string resource_path{};
    // first argument is resource path
    if (argc > 1)
    {
        resource_path = argv[1];
    }
    // no resource path specified, use default
    else
    {
        std::string exe_path{argv[0]};
        resource_path = exe_path.substr(0, exe_path.find_last_of("/\\"));
        resource_path += "/";
    }

    return resource_path;
}


#endif
