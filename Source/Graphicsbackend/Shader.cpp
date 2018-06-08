/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Provides simple shader-management.
*/

#include "../Stdinclude.hpp"

void Shader_t::Select()
{
    glUseProgram(ProgramID);
}
void Shader_t::setInt(const std::string &Name, int Value)
{
    glUniform1i(glGetUniformLocation(ProgramID, Name.c_str()), Value);
}
void Shader_t::setBool(const std::string &Name, bool Value)
{
    glUniform1i(glGetUniformLocation(ProgramID, Name.c_str()), (int)Value);
}
void Shader_t::setFloat(const std::string &Name, float Value)
{
    glUniform1f(glGetUniformLocation(ProgramID, Name.c_str()), Value);
}
Shader_t::Shader_t(const char *Vertexsource, const char *Fragmentsource)
{
    auto Fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    auto Vertexshader = glCreateShader(GL_VERTEX_SHADER);
    ProgramID = glCreateProgram();

    // Supply the sourcecode.
    glShaderSource(Fragmentshader, 1, &Fragmentsource, NULL);
    glShaderSource(Vertexshader, 1, &Vertexsource, NULL);

    // Compile the shaders.
    glCompileShader(Fragmentshader);
    glCompileShader(Vertexshader);

    // Create the program.
    glAttachShader(ProgramID, Fragmentshader);
    glAttachShader(ProgramID, Vertexshader);
    glLinkProgram(ProgramID);

    // Cleanup.
    glDeleteShader(Fragmentshader);
    glDeleteShader(Vertexshader);
}

Shader_t *Graphics::Createshader(const char *Vertexsource, const char *Fragmentsource)
{
    static std::unordered_map<uint64_t, Shader_t *> Cache;

    // Check the cache for pre-compiled shaders.
    uint64_t Hash = Hash::FNV1a_64(Vertexsource) ^ Hash::FNV1a_64(Fragmentsource);
    auto Entry = &Cache[Hash];
    if (*Entry) return *Entry;

    *Entry = new Shader_t(Vertexsource, Fragmentsource);
    return *Entry;
}
