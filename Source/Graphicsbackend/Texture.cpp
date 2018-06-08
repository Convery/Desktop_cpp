/*
    Initial author: Convery (tcn@ayria.se)
    Started: 07-06-2018
    License: MIT

    Provides simple texture-management.
*/

#include "../Stdinclude.hpp"

Texture_t Graphics::Createtexture(RGBA Color)
{
    static std::unordered_map<uint64_t, Texture_t> Cache;

    // Check the cache for pre-created textures.
    uint64_t Hash = Hash::FNV1a_64(&Color, sizeof(Color));
    auto Entry = &Cache[Hash];
    if (*Entry) return *Entry;

    glGenTextures(1, Entry);
    glBindTexture(GL_TEXTURE_2D, *Entry);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float Pixels[] = { Color.r, Color.g, Color.b, Color.a };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, Pixels);

    return *Entry;
}
Texture_t Graphics::Createtexture(uint32_t Width, uint32_t Height, const void *Databuffer, bool Alpha)
{
    static std::unordered_map<uint64_t, Texture_t> Cache;

    // Check the cache for pre-created textures.
    uint64_t Hash = Hash::FNV1a_64(&Databuffer, Width * Height);
    auto Entry = &Cache[Hash];
    if (*Entry) return *Entry;

    glGenTextures(1, Entry);
    glBindTexture(GL_TEXTURE_2D, *Entry);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (!Alpha) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, Databuffer);
    else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Databuffer);

    return *Entry;
}
