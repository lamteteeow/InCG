#pragma once
#include "application.hpp"
#include "helper.hpp"
#include "models.hpp"

#include <glm/glm.hpp>

class Assignment02 : public Application
{
   public:
    // allocate and initialize objects
    Assignment02(std::string const& resource_path);
    Assignment02(Assignment02 const&) = delete;
    Assignment02& operator=(Assignment02 const&) = delete;

    // draw all objects
    void render() override;
    void imgui() override;
    void update(float dt) override{};

   private:
    // common methods
    void initializeShaderPrograms();
    void initializeObjects();

    // special methods
    void renderMap() const;
    void renderObject(GLuint program) const;

    Tex envMap;

    // render objects
    simpleModel teaPot;
    simpleModel sphere;
    simpleModel bunny;

    solidSphere envSphere;

    bool debugUV = false;
    bool useCylindricMapping = false;

    int assignmentNr = 0;
    int envMapID     = 0;
    int objectID     = 0;
    float objectRotation = 0;
    int glossyRays   = 16;
    float roughness  = 0.0f;
};
