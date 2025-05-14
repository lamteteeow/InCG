#pragma once
#include <chrono>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/types.h>
#include <iomanip>
#include <string>
#include <vector>
// use gl definitions from glbinding
using namespace gl;

#include <glm/gtc/type_precision.hpp>

// Texture
class Tex
{
    glm::uvec2 m_dimensions;
    GLuint m_index;

   public:
    Tex(unsigned w, unsigned h, GLenum internal_format);
    Tex(glm::uvec2 const& dims, GLenum internal_format);
    Tex(std::string const& filename, GLenum wrap_mode = GL_REPEAT);
    Tex(Tex&&);
    Tex(Tex const&) = delete;
    Tex& operator   =(Tex&&);
    Tex& operator=(Tex const&) = delete;
    ~Tex();
    void bind() const;
    GLuint index() const;
    glm::uvec2 const& dimensions() const;

    friend void swap(Tex& lhs, Tex& rhs);
};
void swap(Tex& lhs, Tex& rhs);

// Frame Buffer Object
class Fbo
{
    GLuint id;
    std::vector<GLenum> attachment_ids;

   public:
    Fbo();
    Fbo(Fbo const&) = delete;
    Fbo(Fbo&&);
    Fbo& operator=(Fbo&&);
    Fbo& operator=(Fbo const&) = delete;
    ~Fbo();
    void bind() const;
    void addTextureAsColorbuffer(Tex const& img);
    void addTextureAsDepthbuffer(Tex const& img);
    void unbind() const;
    void check() const;

    friend void swap(Fbo& lhs, Fbo& rhs);
};
void swap(Fbo& lhs, Fbo& rhs);

// timer
class Timer
{
   public:
    Timer();
    void update();
    float intervall;

   private:
    std::chrono::system_clock::time_point startTime;
};

// camera stuff
class cameraSystem
{
   public:
    cameraSystem(glm::fvec3 pos);
    cameraSystem(glm::fvec3 pos, glm::fvec3 dir, glm::fvec3 up = glm::fvec3{0.0, 1.0, 0.0});
    cameraSystem(float radius, float phi, float theta);


    void moveForward(float delta);
    void moveBackward(float delta);
    void moveUp(float delta);
    void moveDown(float delta);
    void moveRight(float delta);
    void moveLeft(float delta);
    void yaw(float angle);
    void pitch(float angle);
    void roll(float angle);
    void zoom(float zoom_Factor);
    // for rotation around origin
    void camRotation(float degrees);

    bool use_orbit;
    float orbit_radius;  // orbit_theta, orbit_phi, orbit_radius;

    glm::fvec3 position;  // position-vector
    glm::fvec3 viewDir;   // viewing direction
    glm::fvec3 rightDir;  // right-vector (cross product of viewing- and up-direction)
    glm::fvec3 upDir;     // up-vector

   private:
};


namespace glm
{
inline std::ostream& operator<<(std::ostream& strm, const mat3& m)
{
    std::setprecision(8);
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            strm << std::setw(12) << m[j][i] << " ";
        }
        strm << "\n";
    }
    return strm;
}

inline std::ostream& operator<<(std::ostream& strm, const mat4& m)
{
    std::setprecision(8);
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            strm << std::setw(12) << m[j][i] << " ";
        }
        strm << "\n";
    }
    return strm;
}

}  // namespace glm