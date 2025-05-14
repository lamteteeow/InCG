#include "uniform_upload.hpp"

#include <glbinding/gl/functions.h>
// use gl definitions from glbinding 
using namespace gl;

void glUniform(int loc, int const* i, unsigned cnt) {
  glUniform1iv(loc, cnt, i);
}
void glUniform(int loc, unsigned const* i, unsigned cnt) {
  glUniform1uiv(loc, cnt, i);
}
void glUniform(int loc, float const* i, unsigned cnt) {
  glUniform1fv(loc, cnt, i);
}
void glUniform(int loc, glm::fvec2 const* i, unsigned cnt) {
  glUniform2fv(loc, cnt, (float*)i);
}
void glUniform(int loc, glm::fvec3 const* i, unsigned cnt) {
  glUniform3fv(loc, cnt, (float*)i);
}
void glUniform(int loc, glm::fvec4 const* i, unsigned cnt) {
  glUniform4fv(loc, cnt, (float*)i);
}
void glUniform(int loc, glm::uvec2 const* i, unsigned cnt) {
  glUniform2uiv(loc, cnt, (unsigned*)i);
}
void glUniform(int loc, glm::fmat4 const* i, unsigned cnt) {
  glUniformMatrix4fv(loc, cnt, false, (float*)i);
}

template<>
void glUniform(int loc, bool const& i) {
 glUniform(loc, (int)i);
}
