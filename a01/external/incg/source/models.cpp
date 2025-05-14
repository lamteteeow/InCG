#include <iostream>
#include <map>

// define M_PI on MSVC
#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/gtc/type_ptr.hpp>

#include <glbinding/gl/enum.h>
#include <glbinding/gl/functions.h>
// use gl definitions from glbinding
using namespace gl;

#include "models.hpp"

// screen space quad
simpleQuad::simpleQuad()
    : indices{0, 1, 3, 1, 2, 3}, vertices{glm::vec3(-1, -1, 0),
                                          glm::vec3(1, -1, 0),
                                          glm::vec3(1, 1, 0),
                                          glm::vec3(-1, 1, 0)},
      vbo{0, 0} {

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glEnableVertexAttribArray(0);
  // ==
  glGenBuffers(2, vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3,
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

}

void simpleQuad::draw() const {

  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

simpleQuad::~simpleQuad() { glDeleteBuffers(2, vbo); }

// screen space quad
simplePoint::simplePoint() : vertex{0, 0, 0}, vbo{0} {

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, &vertex, GL_STATIC_DRAW);


  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);


  glBindVertexArray(0);
}

void simplePoint::draw() const {
  glBindVertexArray(vao);
  glDrawArrays(GL_POINTS, 0, 1);
  glBindVertexArray(0);
}

simplePoint::~simplePoint() { glDeleteBuffers(1, &vbo); }

simpleModel::simpleModel() : indices{}, vertices{}, normals{} {}

simpleModel::simpleModel(std::string const &filename) : simpleModel{} {
  std::vector<glm::vec3> normals_tmp;
  std::vector<uint32_t> nIndices;

  FILE *file = fopen(filename.c_str(), "r");
  if (file == NULL) {
    std::cerr << "Model file not found: " << filename << std::endl;
    exit(0);
  }

  while (1) {
    char lineHeader[128];
    int res = fscanf(file, "%s", lineHeader);
    if (res == EOF)
      break;

    if (strcmp(lineHeader, "v") == 0) {
      glm::vec3 vertex;
      fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
      vertices.push_back(vertex);
    } else if (strcmp(lineHeader, "vn") == 0) {
      glm::vec3 normal;
      fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
      normals_tmp.push_back(normal);
    } else if (strcmp(lineHeader, "f") == 0) {
      unsigned vIndex[3], nIndex[3];
      int matches =
          fscanf(file, "%u//%u %u//%u %u//%u\n", &vIndex[0], &nIndex[0],
                 &vIndex[1], &nIndex[1], &vIndex[2], &nIndex[2]);
      if (matches < 6) {
        std::cerr << "Information missin in obj file. " << matches << ", "
                  << lineHeader << std::endl;
        exit(0);
      }

      for (int i = 0; i < 3; ++i) {
        indices.push_back(vIndex[i] - 1);
        nIndices.push_back(nIndex[i] - 1);
      }
    }
  }

  std::vector<glm::vec3> extra_verts;
  std::vector<glm::vec3> extra_norms;
  std::map<std::pair<int, int>, int> mapping;

  normals.resize(vertices.size());
  std::vector<bool> tested(vertices.size(), false);

  for (size_t i = 0; i < indices.size(); ++i) {
    int vIdx = indices[i];
    int nIdx = nIndices[i];

    if (tested[vIdx] && vIdx != nIdx) {
      auto it = mapping.find(std::pair<int, int>(vIdx, nIdx));
      if (it != mapping.end())
        indices[i] = uint32_t(vertices.size() + it->second - 1);
      else {
        extra_verts.push_back(vertices[indices[i]]);
        extra_norms.push_back(normals_tmp[nIndices[i]]);
        indices[i] = uint32_t(vertices.size() + extra_verts.size() - 1);
        mapping[std::pair<int, int>(vIdx, nIdx)] = (int)extra_verts.size();
      }
    } else {
      normals[indices[i]] = normals_tmp[nIndices[i]];
      tested[indices[i]] = true;
    }
  }

  for (auto it = tested.begin(); it != tested.end(); ++it)
    if (*it != true)
      std::cout << "probs" << std::endl;

  vertices.insert(vertices.end(), extra_verts.begin(), extra_verts.end());
  normals.insert(normals.end(), extra_norms.begin(), extra_norms.end());

  mapping.clear();
  extra_verts.clear();
  extra_norms.clear();
  nIndices.clear();
  normals_tmp.clear();

  if (vertices.size() != normals.size()) {
    std::cerr << "Object data (vertices/normals) inconsitent." << std::endl;
    exit(0);
  }

    upload();
}

simpleModel::~simpleModel() { glDeleteBuffers(3, vbo); }

void simpleModel::upload() {
  assert(vao == 0);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(3, vbo);


  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3,
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,  sizeof(float) * 3, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float) * 3,
               normals.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,  sizeof(float) * 3, 0);


  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);






  glBindVertexArray(0);
}

void simpleModel::draw() const {
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
   glBindVertexArray(0);
}

groundPlane::groundPlane(const float height, const float width)
    : simpleModel{} {
  vertices = std::vector<glm::vec3>(4);
  vertices = {glm::vec3(-width, height, -width),
              glm::vec3(-width, height, width), glm::vec3(width, height, width),
              glm::vec3(width, height, -width)};

  normals = std::vector<glm::vec3>(4, glm::vec3(0.f, 1.f, 0.f));

  indices = std::vector<unsigned int>(6);
  indices = {0, 1, 3, 1, 2, 3};

  upload();
}

groundPlane::groundPlane(const float height, const float width,
                         const unsigned int resolution) {
  const float step = 2.f * width / resolution;

  vertices = std::vector<glm::fvec3>();
  indices = std::vector<unsigned int>();

  for (int i = 0; i < resolution; ++i) {
    for (int k = 0; k < resolution; ++k) {
      vertices.push_back(
          glm::fvec3(-width + k * step, height, -width + i * step));
      vertices.push_back(
          glm::fvec3(-width + (k + 1) * step, height, -width + i * step));
      vertices.push_back(
          glm::fvec3(-width + k * step, height, -width + (i + 1) * step));
      vertices.push_back(
          glm::fvec3(-width + (k + 1) * step, height, -width + (i + 1) * step));

      indices.push_back(vertices.size() - 1 - 3);
      indices.push_back(vertices.size() - 1 - 2);
      indices.push_back(vertices.size() - 1 - 1);

      indices.push_back(vertices.size() - 1 - 2);
      indices.push_back(vertices.size() - 1 - 1);
      indices.push_back(vertices.size() - 1);
    }
  }

  normals = std::vector<glm::fvec3>((resolution) * (resolution)*6,
                                    glm::fvec3(0.f, 1.f, 0.f));
  upload();
}

solidTorus::solidTorus(const float r, const float R, const float sides,
                       const float rings)
    : simpleModel{} {
  int i, j;
  float theta, phi, theta1;
  float cosTheta, sinTheta;
  float cosTheta1, sinTheta1;
  float ringDelta, sideDelta;

  ringDelta = 2.0f * (float)M_PI / rings;
  sideDelta = 2.0f * (float)M_PI / sides;

  theta = 0.0;
  cosTheta = 1.0;
  sinTheta = 0.0;

  for (i = (int)rings - 1; i >= 0; i--) {
    theta1 = theta + ringDelta;
    cosTheta1 = glm::cos(theta1);
    sinTheta1 = glm::sin(theta1);
    phi = 0.0;
    for (j = (int)sides; j >= 0; j--) {
      float cosPhi, sinPhi, dist;

      phi += sideDelta;
      cosPhi = glm::cos(phi);
      sinPhi = glm::sin(phi);
      dist = R + r * cosPhi;

      normals.push_back(
          glm::vec3(cosTheta1 * cosPhi, -sinTheta1 * cosPhi, sinPhi));
      vertices.push_back(
          glm::vec3(cosTheta1 * dist, -sinTheta1 * dist, r * sinPhi));

      normals.push_back(
          glm::vec3(cosTheta * cosPhi, -sinTheta * cosPhi, sinPhi));
      vertices.push_back(
          glm::vec3(cosTheta * dist, -sinTheta * dist, r * sinPhi));

      if (j < sides) {
        indices.push_back((uint32_t)vertices.size() - 1 - 2);
        indices.push_back((uint32_t)vertices.size() - 1);
        indices.push_back((uint32_t)vertices.size() - 1 - 1);

        indices.push_back((uint32_t)vertices.size() - 1 - 2);
        indices.push_back((uint32_t)vertices.size() - 1 - 1);
        indices.push_back((uint32_t)vertices.size() - 1 - 3);
      }
    }

    theta = theta1;
    cosTheta = cosTheta1;
    sinTheta = sinTheta1;
  }

  upload();
}

solidSphere::solidSphere(const float r, const int slices, const int stacks)
    : indices{}, uvs{}, vertices{} {

  const float dTheta = 2.0f * (float)M_PI / (float)stacks;
  const float dPhi = (float)M_PI / (float)slices;

  for (int i = stacks; i >= 0; i--) {
    glm::vec2 t(1 - (float)i * dTheta / (2.0 * M_PI), 1.0f);
    glm::vec3 p(0, r, 0);
    vertices.push_back(p);
    uvs.push_back(t);
  }

  auto tmpSize = vertices.size();

  // North pole
  for (int i = stacks; i >= 0; i--) {
    glm::vec2 t(1 - (float)i * dTheta / (2.0 * M_PI), (M_PI - dPhi) / M_PI);
    glm::vec3 p(r * glm::sin(dPhi) * glm::cos((float)i * dTheta),
                r * glm::cos(dPhi),
                r * glm::sin(dPhi) * glm::sin((float)i * dTheta));
    vertices.push_back(p);
    uvs.push_back(t);
  }

  for (int32_t triangleID = 0; triangleID < stacks; triangleID++) {
    indices.push_back((uint32_t)(triangleID));
    indices.push_back((uint32_t)(triangleID + tmpSize));
    indices.push_back((uint32_t)(triangleID + tmpSize + 1));
  }

  indices.push_back(stacks - 1);
  indices.push_back(stacks * 2 - 1);
  indices.push_back(stacks);

  // Middle Part

  //	v0--- v2
  //  |  	/ |
  //  |  /  |
  //  | /   |
  //  v1--- v3

  for (int j = 1; j < slices - 1; j++) {
    for (int i = stacks; i >= 0; i--) {
      glm::vec2 t = glm::vec2(1 - (float)i * dTheta / (2.0 * M_PI),
                              (M_PI - ((float)j + 1) * dPhi) / M_PI);
      glm::vec3 p = glm::vec3(
          r * glm::sin(((float)j + 1) * dPhi) * glm::cos((float)i * dTheta),
          r * glm::cos(((float)j + 1) * dPhi),
          r * glm::sin(((float)j + 1) * dPhi) * glm::sin((float)i * dTheta));
      vertices.push_back(p);
      uvs.push_back(t);

      // add two triangles

      indices.push_back(uint32_t(vertices.size() - stacks - 2)); // v0
      indices.push_back(uint32_t(vertices.size() - 1));          // v1
      indices.push_back(uint32_t(vertices.size() - stacks - 1)); // v2

      indices.push_back(uint32_t(vertices.size() - stacks - 1)); // v2
      indices.push_back(uint32_t(vertices.size() - 1));          // v1
      indices.push_back(uint32_t(vertices.size()));              // v3
    }
  }

  size_t lastVertex = vertices.size() - 1;

  // South Pole
  for (int i = stacks; i >= 0; i--) {
    glm::vec2 t(1 - (float)i * dTheta / (2.0 * M_PI), 0.0f);
    glm::vec3 p = glm::vec3(0, -r, 0);
    vertices.push_back(p);
    uvs.push_back(t);
  }

  for (int32_t triangleID = 0; triangleID < stacks; triangleID++) {
    indices.push_back(uint32_t(lastVertex - stacks + triangleID));
    indices.push_back(uint32_t(lastVertex + triangleID + 1));
    indices.push_back(uint32_t(lastVertex - stacks + triangleID + 1));
  }

  upload();
}

solidSphere::~solidSphere() { glDeleteBuffers(3, vbo); }

void solidSphere::upload() {
  assert(vao == 0);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  // ==
  glGenBuffers(3, vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3,
               vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

  glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
  glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float) * 2, uvs.data(),
               GL_STATIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void solidSphere::draw() const {
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
