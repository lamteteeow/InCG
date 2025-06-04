#ifndef SHADER_LOADER_HPP
#define SHADER_LOADER_HPP

#include <glbinding/gl/enum.h>
#include <map>
#include <string>
using namespace gl;

namespace shader_loader
{
// compile shader
unsigned shader(std::string const& file_path, GLenum shader_type, bool binary = false);
// create program from given list of stages
unsigned program(std::map<GLenum, std::string> const&, bool binary = false);
unsigned program_binary(std::string const& file);


bool RequireReload();
}  // namespace shader_loader

#endif
