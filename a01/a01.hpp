#pragma once

#include "application.hpp"
#include "helper.hpp"
#include "models.hpp"

#include <array>
#include <glm/glm.hpp>
#include <memory>

class Assignment01 : public Application
{
   public:
    // allocate and initialize objects
    Assignment01(std::string const& resource_path);
    Assignment01(Assignment01 const&) = delete;
    Assignment01& operator=(Assignment01 const&) = delete;

    // draw all objects
    void render() override;
    void imgui() override;
    void update(float dt) override;

   protected:
    // common methods
    void initializeShaderPrograms();
    void initializeObjects();
    void resize() override;

    // special methods
    void renderScene() const;
    void taaPass();
    void jitterAndWeight();
    void camRotation();

    // render objects
    simpleQuad quad;
    simpleModel teaPot;
    groundPlane plane;

    // frame buffer object
    Fbo fbo;

    // textures
    Tex color_buffer;
    Tex depth_buffer;
    std::unique_ptr<Tex> taa_in;
    std::unique_ptr<Tex> taa_out;

    glm::fvec3 lightDir;
    glm::fmat4 jittProjMatrix;
    glm::fmat4 prevViewMatrix;

    Timer timer;
    float degreesPerSecond = 20;

    std::array<float, 9> weights;
    float maxFeedback = 0.99f;
    unsigned samples  = 64;

    bool useTaa            = true;
    bool initTaa           = true;
    bool useReprojection   = true;
    bool doFilter          = false;
    bool doClamp           = false;
    bool doDivide2           = false;
    bool doDynamicFeedback = false;
    int zoom              = 1;
    bool freeze            = false;
    bool rotateCam         = false;
};
