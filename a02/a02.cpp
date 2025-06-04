#include "a02.hpp"

#include <glbinding/gl/bitfield.h>
#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
#include <glm/gtx/transform.hpp>

#include "shader_loader.hpp"

void Assignment02::render()
{
    renderMap();
    switch (assignmentNr)
    {
        case 0:
            renderObject(shader("shaderA"));
            break;
        case 1:
            renderObject(shader("shaderB"));
            break;
        case 2:
            glUseProgram(shader("shaderC"));
            uniform("shaderC", "glossyRays", glossyRays);
            uniform("shaderC", "roughness", roughness);
            renderObject(shader("shaderC"));
            break;
    }
}

void Assignment02::renderMap() const
{
    glClearColor(0.2f, 0.2f, 0.2f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader("map"));

    uniform("map", "envMap", 0);
    uniform("map", "debugUV", debugUV);

    uniform("map", "viewMatrix", viewMatrix());
    uniform("map", "projMatrix", projectionMatrix());
    uniform("map", "modelMatrix", glm::mat4(1));

    envSphere.draw();

    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(0);
}

void Assignment02::renderObject(GLuint program) const
{
    glUseProgram(program);

    uniform(program, "envMap", 0);
    uniform(program, "cylindricMapping", useCylindricMapping);
    uniform(program, "debugUV", debugUV);

    uniform(program, "viewMatrix", viewMatrix());
    uniform(program, "projMatrix", projectionMatrix());

    auto R = glm::rotate(objectRotation, glm::fvec3(0, 1, 0));
    auto T = glm::translate(glm::fvec3(0, -0.33, 0));
    switch(objectID){
        case 0: 
            uniform(program, "modelMatrix", T*R*glm::scale(glm::fvec3(1,1,1)));
            sphere.draw();
        break;
        case 1:
            uniform(program, "modelMatrix", T*R*glm::scale(glm::fvec3(2,2,2)));
            teaPot.draw();
        break;
        case 2:
            uniform(program, "modelMatrix", T*R*glm::scale(glm::fvec3(.1f,.1f,.1f)));
            bunny.draw();
        break;
    }

    glUseProgram(0);
}

Assignment02::Assignment02(std::string const& resource_path)
    : Application{resource_path},
      envMap{m_resource_path + "/data/waterfall.png"},
      teaPot{m_resource_path + "/data/teapot.obj"},
      sphere{m_resource_path + "/data/sphere.obj"},
      bunny{m_resource_path + "/data/bunny.obj"},
      envSphere{60, 1000, 1000}
{
    m_cam = cameraSystem{glm::fvec3(1.5f, 1.5f, 1.5f)};
    updateCamera();

    initializeObjects();
    initializeShaderPrograms();
}


// load shader programs
void Assignment02::initializeShaderPrograms()
{
    initializeShader("map", {{GL_VERTEX_SHADER, m_resource_path + "/shader/map.vs.glsl"},
                             {GL_FRAGMENT_SHADER, m_resource_path + "/shader/map.fs.glsl"}});
    initializeShader("shaderA", {{GL_VERTEX_SHADER, m_resource_path + "/shader/shaderA.vs.glsl"},
                                 {GL_FRAGMENT_SHADER, m_resource_path + "/shader/shaderA.fs.glsl"}});
    initializeShader("shaderB", {{GL_VERTEX_SHADER, m_resource_path + "/shader/shaderB.vs.glsl"},
                                 {GL_FRAGMENT_SHADER, m_resource_path + "/shader/shaderB.fs.glsl"}});
    initializeShader("shaderC", {{GL_VERTEX_SHADER, m_resource_path + "/shader/shaderC.vs.glsl"},
                                 {GL_FRAGMENT_SHADER, m_resource_path + "/shader/shaderC.fs.glsl"}});
}

void Assignment02::initializeObjects()
{
    glActiveTexture(GL_TEXTURE0);
    envMap.bind();
}


void Assignment02::imgui()
{
    ImGui::SetNextWindowSize(ImVec2(200, 600), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    if (ImGui::Begin("Assignment02"))
    {
        imGui_plotFPS();
        const char* textures[] = {"waterfall.png", "panorama.png"};
        if (ImGui::Combo("envMap", &envMapID, textures, 2))
        {
            useCylindricMapping = envMapID == 1;
            envMap = Tex(m_resource_path + "/data/" + textures[envMapID]);
            initializeObjects();
        }
        const char* objects[] = {"sphere", "teapot", "bunny"};
        ImGui::Combo("object", &objectID, objects, 3);
        
        ImGui::Checkbox("debugUV", &debugUV);
        ImGui::SliderFloat("objectRotation", &objectRotation, 0, 2 * M_PI);
        ImGui::Separator();
        const char* names[] = {"shaderA", "shaderB", "shaderC"};
        ImGui::Combo("assignment", &assignmentNr, names, 3);
        if (assignmentNr == 2)
        {
            ImGui::SliderFloat("roughness", &roughness, 0, 0.1f);
            ImGui::SliderInt("glossyRays", &glossyRays, 1, 128);
        }
    }
    ImGui::End();
}
// exe entry point
int main(int argc, char* argv[])
{
    std::string resource_path = read_resource_path(argc, argv);
    Assignment02 application{resource_path};
    application.run(60);  // max fps
}
