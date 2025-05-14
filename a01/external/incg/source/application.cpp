#include "application.hpp"

#include <glbinding/gl/gl.h>
#include <iostream>
#include <thread>
// use gl definitions from glbinding
using namespace gl;

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "shader_loader.hpp"

Application::Application(std::string const& resource_path)
    : m_resource_path{resource_path},
      m_cam{glm::vec3(3.0f, 16.0f, 22.f)},
      m_shader_handles{},
      m_shader_files{},
      m_pressed_right{false},
      m_pressed_middle{false},
      m_pressed_left{false},
      m_speed_keyboard{1.0f},
      m_speed_mouse{1.0f},
      m_viewMatrix{},
      m_projMatrix{},
      m_resolution{WIDTH, HEIGHT}
{
    window = initialize(glm::uvec2{WIDTH, HEIGHT}, 3, 2);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Control
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    window_handler::set_callback_object(window, this);
    window_handler::set_key_callback(window,
                                     [](GLFWwindow* w, int a, int b, int c, int d)
                                     {
                                         (void)b;
                                         (void)d;
                                         static_cast<Application*>(glfwGetWindowUserPointer(w))->keyCallback(w, a, c);
                                     });
    window_handler::set_mouse_callback(
        window, [](GLFWwindow* w, double a, double b)
        { static_cast<Application*>(glfwGetWindowUserPointer(w))->cursorCallback(a, b); });
    window_handler::set_button_callback(
        window, [](GLFWwindow* w, int a, int b, int c)
        { static_cast<Application*>(glfwGetWindowUserPointer(w))->buttonCallback(a, b, c); });
    window_handler::set_scroll_callback(
        window, [](GLFWwindow* w, double a, double b)
        { static_cast<Application*>(glfwGetWindowUserPointer(w))->scrollCallback(a, b); });
    window_handler::set_resize_callback(
        window, [](GLFWwindow* w, int a, int b)
        { static_cast<Application*>(glfwGetWindowUserPointer(w))->resizeCallback(a, b); });
    window_handler::set_char_callback(
        window, [](GLFWwindow* w, unsigned int unicode_codepoint)
        { static_cast<Application*>(glfwGetWindowUserPointer(w))->charCallback(unicode_codepoint); });
    window_handler::set_char_mods_callback(
        window, [](GLFWwindow* w, unsigned int unicode_codepoint, int mods)
        { static_cast<Application*>(glfwGetWindowUserPointer(w))->charModsCallback(unicode_codepoint, mods); });



    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();



    glClearDepth(1);
    glClearColor(0.1f, 0.4f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);  // turn on the depth test

    // initialize view and projection matrices
    updateCamera();
    resizeCallback(m_resolution.x, m_resolution.y);

    std::fill(last_frame_times, last_frame_times + last_frame_times_N, 0);
}

Application::~Application()
{
    // free all shader program objects
    for (auto const& pair : m_shader_handles)
    {
        glDeleteProgram(pair.second);
    }

    window_handler::close_and_quit(window, EXIT_SUCCESS);
}

void Application::updateShaderPrograms()
{
    std::cout << "Reloading Shaders" << std::endl;
    for (auto const& pair : m_shader_files)
    {
        auto& handle = m_shader_handles[pair.first];
        try
        {
            auto handle_new = shader_loader::program(pair.second);
            // if compilation throws exception, old handle is not overridden
            glDeleteProgram(handle);
            handle = handle_new;
        }
        catch (std::exception&)
        {
        }
    }
}

uint32_t Application::shader(std::string const& name) const
{
    return m_shader_handles.at(name);
}

void Application::initializeShader(std::string const& name, std::map<GLenum, std::string> const& files, bool binary)
{
    if (binary)
    {
        throw std::runtime_error{"SPIR-V loading through this interface not supported"};
    }
    m_shader_files.emplace(name, files);
    m_shader_handles.emplace(name, shader_loader::program(files, binary));
}

void Application::updateCamera()
{
    const glm::vec3 eye    = glm::vec3(m_cam.position);
    const glm::vec3 center = glm::vec3(m_cam.position + m_cam.viewDir);
    // always keep global up vector
    const glm::vec3 up = glm::vec3(m_cam.upDir);
    m_viewMatrix       = glm::lookAt(eye, center, up);
}

void Application::resizeCallback(int w, int h)
{
    w            = std::max(1, w);  // windowsizes of <= 0 lead to div/0 errors
    h            = std::max(1, h);
    m_resolution = glm::uvec2{w, h};

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    static float aspect_orig = float(WIDTH) / float(HEIGHT);
    float aspect             = float(m_resolution.x) / float(m_resolution.y);
    float fov_y              = 70.0f;
    if (aspect < aspect_orig)
    {
        fov_y = 2.0f * glm::atan(glm::tan(70.0f * 0.5f) * (aspect_orig / aspect));
    }
    // projection is hor+
    m_projMatrix = glm::perspective(fov_y, aspect, 1.0f, 100.0f);
    resize();
}

void Application::resize()
{
    // called during resize callback within constructor
}

void Application::charCallback(unsigned int unicode) {}
void Application::charModsCallback(unsigned int unicode, int mods) {}

void Application::keyCallback(GLFWwindow* window, int key, int action)
{
    if (ImGui::GetIO().WantCaptureKeyboard)
    {
        return;
    }


    switch (key)
    {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_W:
            m_cam.moveForward(m_speed_keyboard);
            updateCamera();
            break;
        case GLFW_KEY_S:
            m_cam.moveBackward(m_speed_keyboard);
            updateCamera();
            break;
        case GLFW_KEY_A:
            m_cam.moveLeft(m_speed_keyboard);
            updateCamera();
            break;
        case GLFW_KEY_D:
            m_cam.moveRight(m_speed_keyboard);
            updateCamera();
            break;
        case GLFW_KEY_Q:
            // m_cam.roll(0.01f*m_speed_mouse);
            updateCamera();
            break;
        case GLFW_KEY_E:
            // m_cam.roll(0.01f*-m_speed_mouse);
            updateCamera();
            break;
        case GLFW_KEY_R:
            updateShaderPrograms();
            break;
        case GLFW_KEY_C:
            const glm::vec3& d = m_cam.viewDir;
            const glm::vec3& u = m_cam.upDir;
            const glm::vec3& p = m_cam.position;
            std::cout << "m_cam-dir: " << d.x << ", " << d.y << ", " << d.z << std::endl
                      << "m_cam-up:  " << u.x << ", " << u.y << ", " << u.z << std::endl
                      << "m_cam-pos: " << p.y << ", " << p.y << ", " << p.z << std::endl;
            break;
    }
}

void Application::buttonCallback(int button, int action, int mods)
{
    (void)mods;
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_PRESS)
                m_pressed_left = true;
            else
                m_pressed_left = false;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS)
                m_pressed_right = true;
            else
                m_pressed_right = false;
            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            if (action == GLFW_PRESS)
                m_pressed_middle = true;
            else
                m_pressed_middle = false;
            break;
    }
}

void Application::cursorCallback(double x, double y)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }
    // vars to track per-frame delta
    static int currentX, currentY;

    const float dx = float(currentX - x);
    const float dy = float(currentY - y);

    currentX = (int)x;
    currentY = (int)y;

    if (m_pressed_right)
    {
        m_cam.moveUp(-dy * 0.03f);
        m_cam.moveRight(dx * 0.03f);
    }
    else if (m_pressed_middle)
    {
        m_cam.moveForward(dy * 0.1f * m_speed_mouse);
        // m_cam.roll(dx*0.001f*m_speed_mouse);
    }
    else if (m_pressed_left)
    {
        float factor = 0.001f;
        if (m_cam.use_orbit)
        {
            factor = 0.005f;
        }

        m_cam.yaw(dx * factor * m_speed_mouse);

        m_cam.pitch(dy * factor * m_speed_mouse);
    }
    // no button pressed -> dont update view matrix
    else
    {
        return;
    }

    updateCamera();
}

void Application::scrollCallback(double offset_x, double offset_y)
{
    (void)offset_x;

    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }

    if (offset_y > 0.0)
    {
        m_speed_keyboard *= (1.0f / 0.75f);
        m_cam.zoom(1.0f / 0.9f);
    }
    else
    {
        m_speed_keyboard *= 0.75f;
        m_cam.zoom(0.9f);
    }
    // std::cout <<m_speed_keyboard << std::endl;

    updateCamera();
}

void Application::run(int max_fps)
{
    using Time = std::chrono::duration<double>;

    Time delta_time = Time(1.0 / max_fps);

    auto start_time = std::chrono::system_clock::now();

    auto current_time = Time(0);
    auto next_update  = current_time;


    int current_query = 0;
    GLuint timer_queries[2];
    glGenQueries(2, &timer_queries[0]);
    for (int i = 0; i < 2; ++i)
    {
        glBeginQuery(GL_TIME_ELAPSED, timer_queries[i]);
        glEndQuery(GL_TIME_ELAPSED);
    }


    // rendering loop
    while (!glfwWindowShouldClose(window))
    {
        if (shader_loader::RequireReload())
        {
            updateShaderPrograms();
        }

        // query input
        glfwPollEvents();
        // draw geometry
        update(delta_time.count());

        glBeginQuery(GL_TIME_ELAPSED, timer_queries[current_query]);
        render();
        glEndQuery(GL_TIME_ELAPSED);

        current_query = (current_query + 1) % 2;


        int done = 0;
        while (!done)
        {
            glGetQueryObjectiv(timer_queries[current_query], GL_QUERY_RESULT_AVAILABLE, &done);
        }

        GLuint64 elapsed_time = 0;
        glGetQueryObjectui64v(timer_queries[current_query], GL_QUERY_RESULT, &elapsed_time);
        last_frame_time = elapsed_time / 1000000.0;
        float last_frame_fp = 1000.0 * 1.0 / last_frame_time;
        last_frame_times[last_frame_times_i % (last_frame_times_N * 2)]                = last_frame_time;
        last_frame_times[(last_frame_times_i + last_frame_times_N) % (last_frame_times_N * 2)] = last_frame_time;
        last_frame_fps[last_frame_times_i % (last_frame_times_N * 2)]                = last_frame_fp;
        last_frame_fps[(last_frame_times_i + last_frame_times_N) % (last_frame_times_N * 2)] = last_frame_fp;
        last_frame_times_i++;



        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        imgui();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // tweak bar
        // TwDraw();
        // swap draw buffer to front
        glfwSwapBuffers(window);

        current_time = std::chrono::duration_cast<Time>(std::chrono::system_clock::now() - start_time);
        next_update += delta_time;

        auto wait_time = next_update - current_time;
        if (wait_time.count() > 0)
        {
            std::this_thread::sleep_for(wait_time);
        }
    }
}

void Application::imGui_plotFPS()
{
    
    //ImGui::Text("Frame Time");  
    //ImGui::SameLine();
    const char* names[] = {"ms", "fps (*theoretical)"};
    ImGui::Combo("Frame Time", &show_ms_fps, names, 2);

    float average = 0;
    int offset = (last_frame_times_i % last_frame_times_N);
    for (int i = 0; i < last_frame_times_N; ++i) average += show_ms_fps==0 ? last_frame_times[ offset + i ] : last_frame_fps[ offset + i ];
    average /= last_frame_times_N;
    std::string average_string = "avg. " + std::to_string(average);

    if(show_ms_fps==0) ImGui::PlotLines("###ms", last_frame_times, last_frame_times_N, offset, average_string.c_str(), 0,  FLT_MAX, ImVec2(ImGui::GetWindowWidth(), 50));
    else               ImGui::PlotLines("###fps", last_frame_fps, last_frame_times_N, offset, average_string.c_str(), 0,  FLT_MAX, ImVec2(ImGui::GetWindowWidth(), 50));
    
}

glm::fmat4 const& Application::viewMatrix() const
{
    return m_viewMatrix;
}
glm::fmat4 const& Application::projectionMatrix() const
{
    return m_projMatrix;
}

glm::uvec2 const& Application::resolution() const
{
    return m_resolution;
}

GLint Application::glGetUniformLocation(GLuint program, const GLchar* name) const
{
    // use function from outer namespace to prevent recursion
    GLint loc = ::glGetUniformLocation(program, name);
    return loc;
    // bool check = false;
    // // if location invalid, output info similar to gl errors
    // if (check && loc == -1) {
    //   std::cerr <<  "OpenGL Error: " << "glGetUniformLocation" << "(";
    //   std::cerr << program << ", " << name << ") - ";
    //   // if no program is bound
    //   if (program == 0) {
    //     std::cerr << "no program bound" << std::endl;
    //     return loc;
    //   }
    //   // a program is bound
    //   std::cerr << name <<" is not an active uniform variable in program ";
    //   for (auto const& pair : m_shader_handles) {
    //     if (pair.second == program) {
    //       std::cerr << '\'' << pair.first << '\'' << std::endl;
    //     }
    //   }
    //   // dont throw, allow retrying
    //   // throw std::runtime_error("Execution of " +
    //   std::string("glGetUniformLocation"));
    //   // exit(EXIT_FAILURE);
    // }
    // return loc;
}
