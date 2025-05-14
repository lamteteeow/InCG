#include "a01.hpp"

// define M_PI on MSVC
#define _USE_MATH_DEFINES
#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <glm/gtx/transform.hpp>
#include <math.h>

#include "shader_loader.hpp"
#include <glm/gtc/matrix_transform.hpp>


void Assignment01::update(float dt)
{
    // when rotation is enabled, compute new angle and update view matrix
    if (rotateCam && !freeze)
    {
        m_cam.camRotation( dt * degreesPerSecond / 180.0f * 3.1415f );
        updateCamera();
    }
}

void Assignment01::render()
{
    // calc jitterd proj and weights
    jitterAndWeight();

    // render scene to temporary buffer
    fbo.bind();
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        renderScene();
    }
    fbo.unbind();

    // compute taa
    if (useTaa)
        taaPass();
    else
        initTaa = true;

    // render quad
    {
        glClearColor(0.2f, 0.2f, 0.2f, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(shader("quad"));

        glActiveTexture(GL_TEXTURE0);
        if (useTaa)
            taa_out->bind();
        else
            color_buffer.bind();

        uniform("quad", "tex", 0);
        uniform("quad", "zoom", zoom);
        uniform("quad", "res", resolution());

        quad.draw();
        glEnable(GL_DEPTH_TEST);
    }
    glUseProgram(0);
}


// 'random' sampler
const float HALTON23[64][2] = {
    {0.0f, -0.333333f},     {-0.5f, 0.333333f},       {0.5f, -0.777778f},     {-0.75f, -0.111111f},
    {0.25f, 0.555556f},     {-0.25f, -0.555556f},     {0.75f, 0.111111f},     {-0.875f, 0.777778f},
    {0.125f, -0.925926f},   {-0.375f, -0.259259f},    {0.625f, 0.407407f},    {-0.625f, -0.703704f},
    {0.375f, -0.037037f},   {-0.125f, 0.62963f},      {0.875f, -0.481481f},   {-0.9375f, 0.185185f},
    {0.0625f, 0.851852f},   {-0.4375f, -0.851852f},   {0.5625f, -0.185185f},  {-0.6875f, 0.481481f},
    {0.3125f, -0.62963f},   {-0.1875f, 0.037037f},    {0.8125f, 0.703704f},   {-0.8125f, -0.407407f},
    {0.1875f, 0.259259f},   {-0.3125f, 0.925926f},    {0.6875f, -0.975309f},  {-0.5625f, -0.308642f},
    {0.4375f, 0.358025f},   {-0.0625f, -0.753086f},   {0.9375f, -0.0864198f}, {-0.96875f, 0.580247f},
    {0.03125f, -0.530864f}, {-0.46875f, 0.135802f},   {0.53125f, 0.802469f},  {-0.71875f, -0.901235f},
    {0.28125f, -0.234568f}, {-0.21875f, 0.432099f},   {0.78125f, -0.679012f}, {-0.84375f, -0.0123457f},
    {0.15625f, 0.654321f},  {-0.34375f, -0.45679f},   {0.65625f, 0.209877f},  {-0.59375f, 0.876543f},
    {0.40625f, -0.82716f},  {-0.09375f, -0.160494f},  {0.90625f, 0.506173f},  {-0.90625f, -0.604938f},
    {0.09375f, 0.0617284f}, {-0.40625f, 0.728395f},   {0.59375f, -0.382716f}, {-0.65625f, 0.283951f},
    {0.34375f, 0.950617f},  {-0.15625f, -0.950617f},  {0.84375f, -0.283951f}, {-0.78125f, 0.382716f},
    {0.21875f, -0.728395f}, {-0.28125f, -0.0617284f}, {0.71875f, 0.604938f},  {-0.53125f, -0.506173f},
    {0.46875f, 0.160494f},  {-0.03125f, 0.82716f},    {0.96875f, -0.876543f}, {-0.984375f, -0.209877f}};


void Assignment01::jitterAndWeight()
{
    // jitter, HALTON23 between -1 and 1
    static int frameId  = 0;
    const float jitterX = HALTON23[frameId][0];
    const float jitterY = HALTON23[frameId][1];
    frameId             = (frameId + 1) % samples;

    // jitter projection
    jittProjMatrix = projectionMatrix();
    // TODO a) setup jittered projection for the current frame (screen space translation)
    // Note the matrix layout if altering the matrix directly

    // Calculate the jitter in normalized device coordinates (NDC)
    float ndcJitterX = jitterX / static_cast<float>(resolution().x);
    float ndcJitterY = jitterY / static_cast<float>(resolution().y);

    // For OpenGL, the projection matrix is column-major.
    // Apply translation in NDC to the projection matrix (offset the [2][0] and [2][1] elements)
    // This shifts the projection center by the jitter amount.
    jittProjMatrix[2][0] += ndcJitterX * 2.0f;
    jittProjMatrix[2][1] += ndcJitterY * 2.0f;


    // TODO b) calculate normalized filter weights (3x3 pixels) for the current frame
    const float sampleOffsets[9][2] = {{-1.0f, -1.0f}, {0.0f, -1.0f}, {1.0f, -1.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f},
                                       {1.0f, 0.0f},   {-1.0f, 1.0f}, {0.0f, 1.0f},  {1.0f, 1.0f}};
    //for (int i = 0; i < 9; ++i) weights[i] = 0;
    //weights[4] = 1;

    float sum = 0.0f;
    for (int i = 0; i < 9; ++i)
    {
        // Using Blackman-Harris 3.3 kernel
        float dx   = sampleOffsets[i][0];
        float dy   = sampleOffsets[i][1];
        weights[i] = std::exp(-2.29f * (dx * dx + dy * dy));
        sum += weights[i];
    }
    // Normalize weights
    for (int i = 0; i < 9; ++i)
    {
        weights[i] /= sum;
    }

}

void Assignment01::renderScene() const
{
    glUseProgram(shader("scene"));
    uniform("scene", "lightDir", lightDir);
    uniform("scene", "viewMatrix", viewMatrix());

    if (useTaa)
        uniform("scene", "projMatrix", jittProjMatrix);
    else
        uniform("scene", "projMatrix", projectionMatrix());

    glm::fvec3 colors[5] = {glm::fvec3(1, 1, 0), glm::fvec3(0.91, 0.54, 0), glm::fvec3(1, 0, 0),
                            glm::fvec3(0.4, 0, 0.91), glm::fvec3(0, 0.65, 1)};

    uniform("scene", "pattern", false);
    for (int i = 0; i < 10; ++i)
    {
        float rad  = ((float)i / 10.f) * (float)M_PI * 2;
        float dist = 8.f;
        float x    = glm::cos(rad) * dist;
        float y    = glm::sin(rad) * dist;

        uniform("scene", "color", colors[i % 5]);
        uniform("scene", "modelMatrix",
                glm::translate(glm::fvec3(x, 0, y)) * glm::scale(glm::fmat4{1.0f}, glm::fvec3(2.5, 2.5, 2.5)));
        teaPot.draw();
    }

    // render plane
    uniform("scene", "pattern", true);
    uniform("scene", "color", glm::fvec3(0.8f, 0.8f, 0.8f));
    uniform("scene", "modelMatrix", glm::fmat4{1.0f});
    plane.draw();

    glUseProgram(0);
}

void Assignment01::taaPass()
{
    std::swap(taa_in, taa_out);

    glUseProgram(shader("taa"));
    GLint work_size[3];
    glGetProgramiv(shader("taa"), GL_COMPUTE_WORK_GROUP_SIZE, work_size);
    unsigned w = resolution().x, h = resolution().y;
    int call_x = (w / work_size[0]) + (w % work_size[0] ? 1 : 0);
    int call_y = (h / work_size[1]) + (w % work_size[1] ? 1 : 0);

    uniform("taa", "init", initTaa);
    initTaa = false;

    uniform("taa", "res", resolution());
    uniform("taa", "weights", weights);

    uniform("taa", "doFilter", doFilter);
    uniform("taa", "doClamp", doClamp);
    uniform("taa", "doDynamicFeedback", doDynamicFeedback);
    uniform("taa", "maxFeedback", maxFeedback);
    uniform("taa", "freeze", freeze);

    glm::fmat4 reProj = glm::fmat4(1);
    if (useReprojection)
    {
        // TODO c) prepare the reprojection matrix - current frame -> previous frame
        
        // The reprojection matrix maps current NDC to previous NDC.
        glm::fmat4 currViewProj = jittProjMatrix * viewMatrix(); // current P V
        glm::fmat4 prevViewProj = jittProjMatrix * prevViewMatrix; // previous P V
        reProj                  = prevViewProj * glm::inverse(currViewProj);

        /////////////////////////////////////////////////////////////////////////
        prevViewMatrix = viewMatrix();
    }

    uniform("taa", "reProj", reProj);

    glActiveTexture(GL_TEXTURE0);
    color_buffer.bind();
    uniform("taa", "currFrame", 0);

    glActiveTexture(GL_TEXTURE1);
    taa_in->bind();
    uniform("taa", "history", 1);

    glBindImageTexture(2, taa_out->index(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute(call_x, call_y, 1);
    glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

    glUseProgram(0);
}


Assignment01::Assignment01(std::string const& resource_path)
    : Application{resource_path},
      teaPot{m_resource_path + "/data/teapot.obj"},
      plane{0.f, 12.f},
      color_buffer{resolution(), GL_RGBA32F},
      depth_buffer{resolution(), GL_DEPTH_COMPONENT32},
      taa_in{new Tex(resolution(), GL_RGBA32F)},
      taa_out{new Tex(resolution(), GL_RGBA32F)},
      lightDir{glm::normalize(glm::fvec3(9.f, -15.f, -10.f))}
{
    initializeObjects();
    initializeShaderPrograms();
}

// load shader programs
void Assignment01::initializeShaderPrograms()
{
    initializeShader("scene", {{GL_VERTEX_SHADER, m_resource_path + "/shader/phong.vs.glsl"},
                               {GL_FRAGMENT_SHADER, m_resource_path + "/shader/phong.fs.glsl"}});
    initializeShader("quad", {{GL_VERTEX_SHADER, m_resource_path + "/shader/quad.vs.glsl"},
                              {GL_FRAGMENT_SHADER, m_resource_path + "/shader/quad.fs.glsl"}});
    initializeShader("taa", {{GL_COMPUTE_SHADER, m_resource_path + "/shader/taa.cs.glsl"}});
}

void Assignment01::initializeObjects()
{
    fbo.bind();
    fbo.addTextureAsColorbuffer(color_buffer);
    fbo.addTextureAsDepthbuffer(depth_buffer);
    fbo.check();
    fbo.unbind();
}

void Assignment01::resize()
{
    initTaa      = true;
    fbo          = Fbo{};
    color_buffer = Tex{resolution(), GL_RGBA32F};
    depth_buffer = Tex{resolution(), GL_DEPTH_COMPONENT32};
    taa_in.reset(new Tex{resolution(), GL_RGBA32F});
    taa_out.reset(new Tex{resolution(), GL_RGBA32F});
    initializeObjects();
}

void Assignment01::imgui()
{
    ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    if (ImGui::Begin("Assignment01"))
    {
        imGui_plotFPS();
        if (ImGui::CollapsingHeader("light"))
        {
            ImGui::Direction("lightDir", lightDir);
        }

        ImGui::Checkbox("rotateCam", &rotateCam);
        if (rotateCam)
        {
            ImGui::SliderFloat("degreesPerSecond", &degreesPerSecond, -180, 180);
        }
        ImGui::Checkbox("freeze", &freeze);
        ImGui::SliderInt("zoom", &zoom, 1, 32, "%d", ImGuiSliderFlags_Logarithmic);
        ImGui::Separator();

        ImGui::Checkbox("useTaa", &useTaa);
        if (useTaa)
        {
            ImGui::Checkbox("useReprojection", &useReprojection);
            ImGui::Checkbox("doFilter", &doFilter);
            ImGui::Checkbox("doClamp", &doClamp);
            ImGui::Checkbox("doDivide2", &doDivide2);
            ImGui::Checkbox("doDynamicFeedback", &doDynamicFeedback);
            ImGui::SliderFloat("maxFeedback", &maxFeedback, 0, 1);
            ImGui::SliderInt("samples", (int*)&samples, 1, 64);
        }
    }
    ImGui::End();
}
// exe entry point
int main(int argc, char* argv[])
{
    std::string resource_path = read_resource_path(argc, argv);
    Assignment01 application{resource_path};
    application.run(30);  // max fps
}
