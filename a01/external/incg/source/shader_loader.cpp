#include "shader_loader.hpp"

#include <fstream>
#include <glbinding/gl/functions.h>
#include <iostream>
#include <regex>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <vector>
#ifdef _WIN32
#    include <windows.h>
#else
#    include <unistd.h>
#endif

#include <unordered_map>
// use gl definitions from glbinding
using namespace gl;

// hidden helper functions, moved from utils
namespace
{
static long getFileWriteTime(const char* name)
{
#ifndef __WIN32
    struct stat sb;
    stat(name, &sb);
    long write;
    write = sb.st_mtime;
    return write;
#else
    ULARGE_INTEGER create, access, write;
    HANDLE hFile   = CreateFileA(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    write.QuadPart = 0;
    if (hFile != INVALID_HANDLE_VALUE)
    {
        GetFileTime(hFile, LPFILETIME(&create), LPFILETIME(&access), LPFILETIME(&write));
    }
    CloseHandle(hFile);
    return write.QuadPart;
#endif
}

std::string file_name(std::string const& file_path)
{
    return file_path.substr(file_path.find_last_of("/\\") + 1);
}
std::string file_path(std::string const& file_path)
{
    return file_path.substr(0, file_path.find_last_of("/\\") + 1);
}

void output_log(GLchar const* log_buffer, std::string const& header)
{
    std::cerr << header << ":\n";
    std::string error{};
    std::istringstream error_stream{log_buffer};
    while (std::getline(error_stream, error))
    {
        std::cerr << " " << error << std::endl;
    }
}

// updated output log (that shows filenames parsed from OpenGL error lines)
std::unordered_map<std::string, int> shader_filenameIDs;

std::vector<std::pair<std::string, long>> shader_filenames;

void shader_error_log(GLchar const* log_buffer, std::string const& header)
{
    std::cerr << header << ":\n";
    std::string error{};
    std::istringstream error_stream{log_buffer};
    int currentID = 0;
    if (shader_filenameIDs.find(header) != shader_filenameIDs.end())
    {
        currentID = shader_filenameIDs[header];
    }

    // TODO:: different OpenGL implementation may result in different
    // error-information logs
    auto reg = std::regex("^([0-9]+)\\([0-9]+\\)\\s+:\\s+.*");
    while (std::getline(error_stream, error))
    {
        std::smatch sm;
        if (std::regex_match(error, sm, reg))
        {
            int id = std::stoi(sm[1]);
            if (id != currentID && id < shader_filenames.size())
            {
                std::cerr << shader_filenames[id].first << ":" << std::endl;
                currentID = id;
            }
        }
        std::cerr << " " << error << std::endl;
    }
}

static std::string read_file(std::string const& name)
{
    std::ifstream ifile(name);
    // manage shader includes:
    std::regex include_regex = std::regex("^#pragma\\s+incg_include\\s+\"(.*)\".*\\s*");
    auto filename            = file_name(name);
    if (shader_filenameIDs.find(filename) == shader_filenameIDs.end())
    {
        shader_filenameIDs[filename] = shader_filenameIDs.size();
        shader_filenames.emplace_back(name, getFileWriteTime(name.c_str()));
    }
    int fileID = shader_filenameIDs[filename];

    shader_filenames[fileID].second = getFileWriteTime(name.c_str());

    if (ifile)
    {
        std::string filetext;
        int lineID = 0;

        while (ifile.good())
        {
            std::string line;
            std::getline(ifile, line);
            // handle CRLF line endings on unix systems
            line = std::regex_replace(line, std::regex("\r+"), "");

            // assert includes starting at correct linenumber (after #version)
            if (lineID == 0) line.append("\n#line 2 ").append(std::to_string(fileID));

            // manage shader includes
            std::smatch match;
            if (std::regex_match(line, match, include_regex))
            {
                std::stringstream stream;
                stream << file_path(name) << match[1];
                std::string inc_file = stream.str();

                std::stringstream stream2;
                stream2 << read_file(inc_file) << "\n#line " << lineID << " " << fileID << "\n";
                line = stream2.str();
            }

            filetext.append(line + "\n");
            lineID++;
        }

        // std::cout<<filetext<<std::endl;
        return filetext;
    }
    else
    {
        std::cerr << "File \'" << name << "\' not found" << std::endl;

        throw std::invalid_argument(name);
    }
}
// http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
std::string read_binary(std::string const& name)
{
    std::ifstream ifile(name, std::ios::in | std::ios::binary);

    if (ifile)
    {
        std::string contents;
        ifile.seekg(0, std::ios::end);
        contents.resize(ifile.tellg());
        ifile.seekg(0, std::ios::beg);
        ifile.read(&contents[0], contents.size());
        ifile.close();
        return contents;
    }
    else
    {
        std::cerr << "File \'" << name << "\' not found" << std::endl;
        throw std::invalid_argument(name + ", errno " + strerror(errno));
    }
}

}  // namespace

namespace shader_loader
{
GLuint shader(std::string const& file_path, GLenum shader_type, bool binary)
{
    GLuint shader = 0;
    shader        = glCreateShader(shader_type);

    if (binary)
    {
        std::string shader_source{read_binary(file_path)};
        glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, shader_source.data(),
                       (GLsizei)(shader_source.size() * sizeof(std::string::value_type)));
        glSpecializeShader(shader, "main", 0, nullptr, nullptr);
    }
    else
    {
        std::string shader_source{read_file(file_path)};
        // glshadersource expects array of c-strings
        const char* shader_chars = shader_source.c_str();
        glShaderSource(shader, 1, &shader_chars, 0);

        glCompileShader(shader);
    }

    // check if compilation was successfull
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == 0)
    {
        // get log length
        GLint log_size = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
        // get log
        GLchar* log_buffer = (GLchar*)malloc(sizeof(GLchar) * log_size);
        glGetShaderInfoLog(shader, log_size, &log_size, log_buffer);
        // output errors
        shader_error_log(log_buffer, file_name(file_path));
        // free broken shader
        glDeleteShader(shader);
        free(log_buffer);

        throw std::logic_error("OpenGL error: compilation of " + file_path);
    }

    return shader;
}

unsigned program(std::map<GLenum, std::string> const& stages, bool spirv)
{
    // detect if input is single binary
    auto bin_stage = stages.find(GL_PROGRAM_BINARY_FORMATS);
    if (bin_stage != stages.end())
    {
        if (stages.size() < 1)
        {
            throw std::runtime_error("Compilation of seperate binary modules not supported");
        }
        return program_binary(bin_stage->second);
    }

    unsigned program = glCreateProgram();

    std::vector<GLuint> shaders{};
    // load and compile vert and frag shader
    for (auto const& stage : stages)
    {
        GLuint shader_handle = shader(stage.second, stage.first, spirv);
        shaders.push_back(shader_handle);
        // attach the shader to program
        glAttachShader(program, shader_handle);
    }
    // make programs retrieveable by default
    glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
    // link shaders
    glLinkProgram(program);

    // check if linking was successfull
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == 0)
    {
        // get log length
        GLint log_size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
        // get log
        GLchar* log_buffer = (GLchar*)malloc(sizeof(GLchar) * log_size);
        glGetProgramInfoLog(program, log_size, &log_size, log_buffer);

        // output errors
        std::string paths{};
        for (auto const& stage : stages)
        {
            paths += file_name(stage.second) + " & ";
        }
        paths.resize(paths.size() - 3);
        output_log(log_buffer, paths);
        // free broken program
        glDeleteProgram(program);
        free(log_buffer);

        throw std::logic_error("OpenGL error: linking of " + paths);
    }

    for (auto shader_handle : shaders)
    {
        // detach shader
        glDetachShader(program, shader_handle);
        // and free it
        glDeleteShader(shader_handle);
    }

    return program;
}
unsigned program_binary(std::string const& file_path)
{
    unsigned program = glCreateProgram();

    std::string shader_binary{read_binary(file_path)};

    int32_t num_formats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num_formats);
    std::vector<GLenum> formats(num_formats);
    glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, formats.data());

    // load binary, use first available format
    glProgramBinary(program, formats.front(), shader_binary.data(), GLsizei(shader_binary.size()));

    // check if linking was successfull
    GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == 0)
    {
        // get log length
        GLint log_size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_size);
        // get log
        GLchar* log_buffer = (GLchar*)malloc(sizeof(GLchar) * log_size);
        glGetProgramInfoLog(program, log_size, &log_size, log_buffer);

        // output errors
        std::string name{file_name(file_path)};
        output_log(log_buffer, name);
        // free broken program
        glDeleteProgram(program);
        free(log_buffer);

        throw std::logic_error("OpenGL error: loading of " + name);
    }

    return program;
}
bool RequireReload()
{
    for (auto& f : shader_filenames)
    {
        auto new_time = getFileWriteTime(f.first.c_str());
        if( new_time != f.second)
        {
            return true;
        }
    }
    return false;
}

}  // namespace shader_loader
