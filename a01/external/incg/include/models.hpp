#pragma once

#include <string>
#include <vector>

#include <glm/gtc/type_precision.hpp>

// Screen Space Quad
class simpleQuad {
public:
  simpleQuad();
  ~simpleQuad();

  simpleQuad(const simpleQuad&) = delete;
  void upload();
  void draw() const;

protected:
  std::vector<uint32_t> indices;
  std::vector<glm::vec3> vertices;
  uint32_t vbo[2];
  GLuint vao = 0;
};

class simplePoint {
public:
  simplePoint();
  ~simplePoint();
  simplePoint(const simplePoint&) = delete;
  void draw() const;

protected:
  glm::vec3 vertex;
  uint32_t vbo;
  GLuint vao = 0;
};

// very simple geometry
class simpleModel {
public:
  simpleModel(std::string const &fileName);
  ~simpleModel();
  simpleModel(const simpleModel&) = delete;
  void draw() const;

protected:
  simpleModel();
  void upload();
  std::vector<uint32_t> indices;
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  uint32_t vbo[3];
  GLuint vao = 0;
};

class groundPlane : public simpleModel {
public:
  groundPlane(const float height, const float width);
  groundPlane(const float height, const float width,
              const unsigned int resolution);
  groundPlane(const groundPlane&) = delete;
};

class solidTorus : public simpleModel {
public:
  solidTorus(const float r, const float R, const float sides,
             const float rings);
};

class solidSphere {
public:
  solidSphere(const float radius, const int slices, const int stacks);
  ~solidSphere();
  solidSphere(const solidSphere&) = delete;
  void draw() const;

protected:
  void upload();
  std::vector<uint32_t> indices;
  std::vector<glm::vec2> uvs;
  std::vector<glm::vec3> vertices;
  uint32_t vbo[3];
  GLuint vao = 0;
};
