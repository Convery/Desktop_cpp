/*
    Initial author: Convery (tcn@ayria.se)
    Started: 24-08-2018
    License: MIT

    Converts code into pretty colors.
*/

#include "../Stdinclude.hpp"

/*
    Split the screen into 8 * 8 areas.
    Render each when dirty.
*/

namespace Engine::Rendering
{
    uint32_t Gridwidth{}, Gridheight{};
    std::bitset<8 * 8> Dirtygrid{};
    point4_t Clippingarea{};
    uint32_t Bufferlength{};
    void *Renderbuffer{};

    // Platform-specific presentation.
    namespace
    {
        #if !defined(_WIN32)
        #error Non-windows abstraction is not implemented (yet!)

        ainline void Present() {}
        ainline void Lockbuffer() {}
        ainline void Unlockbuffer() {}
        ainline pixel32_t fromRGBA(const rgba_t Color)
        {
            return
            {
                uint8_t(Color.R <= 1 ? Color.R * 255 : Color.R),
                uint8_t(Color.G <= 1 ? Color.G * 255 : Color.G),
                uint8_t(Color.B <= 1 ? Color.B * 255 : Color.B),
                uint8_t(Color.A <= 1 ? Color.A * 255 : Color.A)
            };
        }
        ainline void _Recalculatebuffers() {}
        #else

        PAINTSTRUCT State{};
        void *Devicecontext{};
        BITMAPINFO Lineformat{ { sizeof(BITMAPINFO), 1, 1, 1, 24 } };
        ainline void Present()
        {
            // Bitblt to screen.

            SetDIBitsToDevice((HDC)Devicecontext, Clippingarea.x0, Clippingarea.y0, Lineformat.bmiHeader.biWidth, -Lineformat.bmiHeader.biHeight, 0, 0, 0, -Lineformat.bmiHeader.biHeight, Renderbuffer, &Lineformat, DIB_RGB_COLORS);
        }
        ainline void Lockbuffer()
        {
            InvalidateRect(HWND(gWindowhandle), NULL, NULL);
            Devicecontext = BeginPaint(HWND(gWindowhandle), &State);
        }
        ainline void Unlockbuffer()
        {
            EndPaint(HWND(gWindowhandle), &State);
        }
        ainline pixel32_t fromRGBA(const rgba_t Color)
        {
            return
            {
                uint8_t(Color.B <= 1 ? Color.B * 255 : Color.B),
                uint8_t(Color.G <= 1 ? Color.G * 255 : Color.G),
                uint8_t(Color.R <= 1 ? Color.R * 255 : Color.R),
                uint8_t(Color.A <= 1 ? Color.A * 255 : Color.A)
            };
        }
        ainline void _Recalculatebuffers()
        {
            // Split the window into a grid.
            Gridheight = gWindowsize.y / 8;
            Gridwidth = gWindowsize.x / 8;

            // The buffer for the pixels should be a multiple of 16.
            Bufferlength = Gridwidth * Gridheight * sizeof(pixel24_t);
            Bufferlength += Bufferlength % 16 == 0 ? 0 : 16 - (Bufferlength % 16);

            // Initialize the scanline info for this pass.
            Lineformat.bmiHeader.biHeight = -Gridheight;
            Lineformat.bmiHeader.biWidth = Gridwidth;

            // Mark all areas as dirty.
            Dirtygrid.set();
        }
        #endif
    }

    // onWindowresize().
    void Recalculatebuffers() { _Recalculatebuffers(); }

    // Mark a region as dirty.
    void Invalidateregion(const point4_t Area)
    {
        for (size_t Y = Area.y0 / Gridheight; Y < Area.y1 / Gridheight; ++Y)
        {
            for (size_t X = Area.x0 / Gridwidth; X < Area.x1 / Gridwidth; ++X)
            {
                Dirtygrid.set(Y * 8 + X);
            }
        }
    }

    // Process elements, render, and present to the context.
    void onFrame()
    {
        // Notify the system that we own the framebuffer.
        Lockbuffer();

        // Create the buffer for this instance.
        Renderbuffer = (uint8_t *)alloca(Bufferlength);

        // Render all the scanlines in the main-thread.
        for (int16_t i = 0; i < 8 * 8; ++i)
        {
            // Skip clean areas.
            if (likely(Dirtygrid[i] == false)) continue;

            // Clear the area to fully transparent (chroma-key on 0xFFFFFF).
            std::memset(Renderbuffer, 0xFF, Bufferlength);
            Clippingarea =
            {
                int16_t(i % 8 * Gridwidth), int16_t(i / 8 * Gridheight),
                int16_t((1 + i % 8) * Gridwidth), int16_t((1 + i / 8) * Gridheight)
            };

            // Helper to save my fingers.
            std::function<void(const Element_t *)> Render = [&](const Element_t *This) -> void
            {
                // Occlusion checking.
                if (This->Dimensions.y0 > Clippingarea.y1 || This->Dimensions.y1 < Clippingarea.y0) return;
                if (This->Dimensions.x0 > Clippingarea.x1 || This->Dimensions.x1 < Clippingarea.x0) return;

                if (This->onRender) This->onRender(This);
                for (const auto &Item : This->Childelements) Render(Item);
            };

            // Render all elements.
            assert(gRootelement);
            Render(gRootelement);

            // Bitblt to screen.
            Present();
        }

        // Reset the area.
        Dirtygrid.reset();

        // Give the framebuffer back to the system.
        Unlockbuffer();
    }

    // Internal setters, should be optimized further. Probably via SSE.
    ainline void setPixel(const size_t Offset, const pixel32_t Color)
    {
        // Alpha is always the last entry.
        if (Color.Raw[3] == 0xFF)
        {
            std::memcpy(((pixel24_t *)Renderbuffer)[Offset].Raw, Color.Raw, sizeof(pixel24_t));
        }
        else
        {
            #define BLEND(A, B) A += int32_t((((B - A) * Color.Raw[3]))) >> 8;
            BLEND(((pixel24_t *)Renderbuffer)[Offset].Raw[0], Color.Raw[0]);
            BLEND(((pixel24_t *)Renderbuffer)[Offset].Raw[1], Color.Raw[1]);
            BLEND(((pixel24_t *)Renderbuffer)[Offset].Raw[2], Color.Raw[2]);
        }
    }
    ainline void setPixel(const size_t Offset, const pixel24_t Color)
    {
        return setPixel(Offset, { Color.Raw[0], Color.Raw[1], Color.Raw[2], 0xFF });
    }

    // Fill an area of the buffer.
    ainline void fillRect(const point4_t Area, const rgba_t Color)
    {
        const auto Pixel{ fromRGBA(Color) };

        // Get the drawable area.
        const point4_t Rect =
        {
            std::clamp(Area.x0, Clippingarea.x0,  Clippingarea.x1),
            std::clamp(Area.y0, Clippingarea.y0,  Clippingarea.y1),
            std::clamp(Area.x1, Clippingarea.x0,  Clippingarea.x1),
            std::clamp(Area.y1, Clippingarea.y0,  Clippingarea.y1)
        };

        // If we don't have any work, return.
        if (Rect.y1 - Rect.y0 <= 0 || Rect.x1 - Rect.x0 <= 0) return;

        // For each pixel.
        for (int16_t Y = Rect.y0; Y < Rect.y1; ++Y)
        {
            for (int16_t X = Rect.x0; X < Rect.x1; ++X)
            {
                setPixel((Y - Clippingarea.y0) * Gridwidth + (X - Clippingarea.x0), Pixel);
            }
        }
    }
    ainline void fillRect(const point4_t Area, const texture_t Texture)
    {
        // Get the drawable area.
        const point4_t Rect =
        {
            std::clamp(Area.x0, Clippingarea.x0,  Clippingarea.x1),
            std::clamp(Area.y0, Clippingarea.y0,  Clippingarea.y1),
            std::clamp(Area.x1, Clippingarea.x0,  Clippingarea.x1),
            std::clamp(Area.y1, Clippingarea.y0,  Clippingarea.y1)
        };

        // If we don't have any work, return.
        if (Rect.y1 - Rect.y0 <= 0 || Rect.x1 - Rect.x0 <= 0) return;

        // For each pixel.
        for (int16_t Y = Rect.y0; Y < Rect.y1; ++Y)
        {
            for (int16_t X = Rect.x0; X < Rect.x1; ++X)
            {
                if (Texture.Pixelsize == sizeof(pixel32_t))
                {
                    setPixel((Y - Clippingarea.y0) * Gridwidth + (X - Clippingarea.x0), ((pixel32_t *)Texture.Data)[(Y - Area.y0) * Texture.Dimensions.x + X - Area.x0]);
                }
                else
                {
                    setPixel((Y - Clippingarea.y0) * Gridwidth + (X - Clippingarea.x0), ((pixel24_t *)Texture.Data)[(Y - Area.y0) * Texture.Dimensions.x + X - Area.x0]);
                }
            }
        }
    }
}

namespace Engine::Rendering::Draw
{
    template <bool Outline> void Quad(const texture_t Color, const point4_t Area)
    {
        fillRect(Area, Color);
    }
    template <bool Outline> void Quad(const rgba_t Color, const point4_t Area)
    {
        if (Outline)
        {
            fillRect({ Area.x0, Area.y0, Area.x1, Area.y0 + 1 }, Color);
            fillRect({ Area.x0, Area.y1 - 1, Area.x1, Area.y1 }, Color);
            fillRect({ Area.x0, Area.y0, Area.x0 + 1, Area.y1 }, Color);
            fillRect({ Area.x1 - 1, Area.y0, Area.x1, Area.y1 }, Color);
        }
        else
        {
            fillRect(Area, Color);
        }
    }
}

/*
    NOTE(Convery):

    So apparently CL refuses to instantiate our templates if they are created in other modules.
    So we need to touch them once in this module so that the linker has something to poke at.
    Don't ask me why, just roll with it.
*/
void Microsoft_hackery_do_not_call_this()
{
    //Draw::Circle<false>(texture_t(), point2_t(), float());
    //Draw::Circle<true>(texture_t(), point2_t(), float());
    //Draw::Circle<false>(rgba_t(), point2_t(), float());
    //Draw::Circle<true>(rgba_t(), point2_t(), float());
    Draw::Quad<false>(texture_t(), point4_t());
    Draw::Quad<true>(texture_t(), point4_t());
    Draw::Quad<false>(rgba_t(), point4_t());
    Draw::Quad<true>(rgba_t(), point4_t());
}

/*
    NOTE(Convery):

    Code to create a texture from a png.
    Needs to be modified for different platforms.
*/
#if !defined(NDEBUG)
#define STB_IMAGE_IMPLEMENTATION
#include "../Utility/stb_image.h"
void PNGtoTexture(std::string Filename)
{
    // Load the image into memory.
    int Imagewidth, Imageheight, Imagechannels;
    const auto Image = stbi_load(va("../Assets/%s.png", Filename.c_str()).c_str(), &Imagewidth, &Imageheight, &Imagechannels, 0);
    assert(Image);

    if (auto Filehandle = std::fopen(va("../Assets/%s.cpp", Filename.c_str()).c_str(), "wb"))
    {
        std::fprintf(Filehandle, "#include \"../Assets.hpp\"\n\n");
        std::fprintf(Filehandle, "namespace Assets {\n");
        std::fprintf(Filehandle, "static const uint8_t Assetdata[] = {\n\t");

        if (Imagechannels == 3)
        {
            auto Pixels = (pixel24_t *)Image;
            for (size_t i = 0; i < Imagewidth * Imageheight; ++i)
            {
                std::fprintf(Filehandle, "0x%x, 0x%x, 0x%x, ", Pixels[i].RGB.B, Pixels[i].RGB.G, Pixels[i].RGB.R);
                if (i % 10 == 0) std::fprintf(Filehandle, "\n\t");
            }
        }
        else if (Imagechannels == 4)
        {
            auto Pixels = (pixel32_t *)Image;
            for (size_t i = 0; i < Imagewidth * Imageheight; ++i)
            {
                std::fprintf(Filehandle, "0x%x, 0x%x, 0x%x, 0x%x, ", Pixels[i].RGBA.B, Pixels[i].RGBA.G, Pixels[i].RGBA.R, Pixels[i].RGBA.A);
                if (i % 10 == 0) std::fprintf(Filehandle, "\n\t");
            }
        }
        else assert(false);

        std::fprintf(Filehandle, "\n};\n");
        std::fprintf(Filehandle, "extern const texture_t %s\n{\n\t{ %u, %u },\n\t%u,\n\tAssetdata\n};\n}\n", Filename.c_str(), Imagewidth, Imageheight, Imagechannels);

        std::fclose(Filehandle);
    }
};
#endif
