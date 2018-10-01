#pragma once
#include "../Stdinclude.hpp"

namespace Assets
{
    extern const texture_t Closeicon;
    extern const texture_t Emailprompt;
    extern const texture_t Github;
    extern const texture_t Google;
    extern const texture_t Loginbackground;
    extern const texture_t Longline;
    extern const texture_t Offlinetext;
    extern const texture_t Registertext;
    extern const texture_t Settingsicon;
    extern const texture_t Shortline;
    extern const texture_t Signintext;
    extern const texture_t Twitter;
}

#define Centertexture(Box, Texture)                         \
{                                                           \
    Box.x0 + (Box.x1 - Box.x0 - Texture.Dimensions.x) / 2,  \
    Box.y0 + (Box.y1 - Box.y0 - Texture.Dimensions.y) / 2,  \
    Box.x1 - (Box.x1 - Box.x0 - Texture.Dimensions.x) / 2,  \
    Box.y1 - (Box.y1 - Box.y0 - Texture.Dimensions.y) / 2   \
}                                                           \
