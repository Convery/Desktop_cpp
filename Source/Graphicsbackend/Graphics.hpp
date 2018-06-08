/*
    Initial author: Convery (tcn@ayria.se)
    Started: 08-06-2018
    License: MIT

    Provides very simple rendering.
*/

#pragma once
#include <cstdint>
#include <cstring>

// Common data-types.
struct vec3 { float x, y, z; };
struct vec4 { float x, y, z, w; };
struct RGBA { float r, g, b, a; };
struct Rect { double x0, y0, x1, y1; };

// Common graphics-types.
using Texture_t = uint32_t;
class Shader_t
{
    unsigned int ProgramID;

    public:
    void Select();
    void setInt(const std::string &Name, int Value);
    void setBool(const std::string &Name, bool Value);
    void setFloat(const std::string &Name, float Value);

    Shader_t(const char *Vertexsource, const char *Fragmentsource);
};

namespace Graphics
{
    // Manage shaders.
    Shader_t *Createshader(const char *Vertexsource, const char *Fragmentsource);

    // Manage textures.
    Texture_t Createtexture(RGBA Color);
    Texture_t Createtexture(uint32_t Width, uint32_t Height, const void *Databuffer, bool Alpha);
}
