/*
    Initial author: Convery (tcn@ayria.se)
    Started: 24-08-2018
    License: MIT

    Converts code into pretty colors.
*/

#include "../Stdinclude.hpp"

void PNGtoTexture(std::string Filename);

namespace Engine
{
    std::vector<bool> Dirtylines{};
    uint32_t Scanlinelength{};
    uint32_t Bufferlength{};
    point4_t Clippingarea{};
    uint8_t *Scanline{};

    int16_t Currentline = 0;

    void setScanlinelength(uint32_t Length)
    {
        // The length of each line.
        Scanlinelength = Length;

        // The length of the buffer for 10 lines.
        Length *= sizeof(pixel24_t) * 10;

        // Align to 16 bytes.
        Bufferlength =  Length + (Length % 16 == 0 ? 0 : 16 - (Length % 16));

        // Ensure that we have enough storage.
        Dirtylines.resize(gWindowsize.y + 1);
    }
}

namespace Engine::Rendering
{
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
        #else

        PAINTSTRUCT State{};
        void *Devicecontext{};
        ainline void Present()
        {
            // Initialize the scanline info for this pass and BitBlt.
            BITMAPINFO Lineformat{ { sizeof(BITMAPINFO), Scanlinelength, -(Clippingarea.y1 - Clippingarea.y0), 1, 24 } };
            SetDIBitsToDevice((HDC)Devicecontext, 0, Clippingarea.y0, Lineformat.bmiHeader.biWidth, -Lineformat.bmiHeader.biHeight, 0, 0, 0, -Lineformat.bmiHeader.biHeight, Scanline, &Lineformat, DIB_RGB_COLORS);
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
        #endif
    }

    // Mark a span of lines as dirty.
    void Invalidatespan(point2_t Span)
    {
        // Normalize the span to prevent out-of-range errors.
        for (int16_t i = std::clamp(Span.first, int16_t(0), gWindowsize.y); i <= std::clamp(Span.second, int16_t(0), gWindowsize.y); ++i)
        {
            Dirtylines[i] = 1;
        }
    }

    // Process elements, render, and present to the context.
    void onFrame()
    {
        // Notify the system that we own the framebuffer.
        Lockbuffer();

        // Create the buffer for this instance.
        Scanline = (uint8_t *)alloca(Bufferlength);

        // Render all the scanlines in the main-thread.
        for (int16_t i = 0; i < gWindowsize.y; ++i)
        {
            // Skip clean lines.
            if (likely(Dirtylines[i] == 0)) continue;

            // Clear the line to fully transparent (chroma-key on 0xFFFFFF).
            Clippingarea = { 0, i, int16_t(Scanlinelength), std::min(gWindowsize.y, int16_t(i + 10)) };
            i += std::min(gWindowsize.y, int16_t(i + 8)) - i;
            std::memset(Scanline, 0xFF, Bufferlength);

            // Helper to save my fingers.
            std::function<void(const Element_t *)> Render = [&](const Element_t *This) -> void
            {
                // Occlusion checking.
                if (This->Dimensions.y0 > Clippingarea.y1 || This->Dimensions.y1 < Clippingarea.y0)
                    return;

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
        Dirtylines.clear();
        Dirtylines.resize(gWindowsize.y);

        // Give the framebuffer back to the system.
        Unlockbuffer();
    }

    // Internal setters, should be optimized further. Probably via SSE.
    ainline void setPixel(const size_t Offset, const pixel32_t Color)
    {
        // Alpha is always the last entry.
        if (Color.Raw[3] == 0xFF)
        {
            std::memcpy(((pixel24_t *)Scanline)[Offset].Raw, Color.Raw, sizeof(pixel24_t));
        }
        else
        {
            #define BLEND(A, B) A += int32_t((((B - A) * Color.Raw[3]))) >> 8;
            BLEND(((pixel24_t *)Scanline)[Offset].Raw[0], Color.Raw[0]);
            BLEND(((pixel24_t *)Scanline)[Offset].Raw[1], Color.Raw[1]);
            BLEND(((pixel24_t *)Scanline)[Offset].Raw[2], Color.Raw[2]);
        }
    }
    ainline void setPixel(const size_t Offset, const pixel24_t Color)
    {
        return setPixel(Offset, { Color.Raw[0], Color.Raw[1], Color.Raw[2], 0xFF });
    }
    ainline void BitBlt(const point4_t Source, const texture_t Texture)
    {
        // Get the drawable area.
        const point4_t Area =
        {
            std::clamp(Source.x0, int16_t(0), int16_t(Scanlinelength)),
            std::clamp(Source.y0, Clippingarea.y0, Clippingarea.y1),
            std::clamp(Source.x1, int16_t(0),  std::min(int16_t(Scanlinelength), int16_t(Source.x0 + Texture.Dimensions.x))),
            std::clamp(Source.y1, Clippingarea.y0, std::min(Clippingarea.y1, int16_t(Source.y0 + Texture.Dimensions.y)))
        };

        // If we don't have any work, return.
        if (Area.y1 - Area.y0 <= 0 || Area.x1 - Area.x0 <= 0) return;

        // For each line to draw.
        for (int16_t Y = Area.y0; Y < Area.y1; ++Y)
        {
            for (int16_t X = Area.x0; X < Area.x1; ++X)
            {
                if (Texture.Pixelsize == sizeof(pixel32_t))
                {

                    setPixel((Y - Area.y0) * Scanlinelength + X, ((pixel32_t *)Texture.Data)[(Y - Source.y0) * Texture.Dimensions.x + X - Source.x0]);
                }
                else
                {
                    setPixel((Y - Area.y0) * Scanlinelength + X, ((pixel24_t *)Texture.Data)[(Y - Source.y0) * Texture.Dimensions.x + X - Source.x0]);
                }
            }
        }
    }
    ainline void BitBlt(const point4_t Source, const rgba_t Color)
    {
        const auto Pixel{ fromRGBA(Color) };

        // Get the drawable area.
        const point4_t Area =
        {
            std::clamp(Source.x0, int16_t(0), int16_t(Scanlinelength)),
            std::clamp(Source.y0, Clippingarea.y0, Clippingarea.y1),
            std::clamp(Source.x1, int16_t(0), int16_t(Scanlinelength)),
            std::clamp(Source.y1, Clippingarea.y0, Clippingarea.y1)
        };

        // If we don't have any work, return.
        if (Area.y1 - Area.y0 <= 0 || Area.x1 - Area.x0 <= 0) return;

        // For each line to draw.
        for (int16_t Y = Area.y0; Y < Area.y1; ++Y)
        {
            for (int16_t X = Area.x0; X < Area.x1; ++X)
            {
                setPixel((Y - Area.y0) * Scanlinelength + X, Pixel);
            }
        }
    }
}

namespace Engine::Rendering::Draw::Internal
{


    // Outline and fill quads.
    template <typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void fillQuad(const point4_t Area, const CB Callback)
    {
        if (Currentline >= Area.y0 && Currentline < Area.y1)
        {
            Callback({ Area.x0, Currentline - Area.y0 }, Area.x1 - Area.x0 + 1);
        }
    }
    template <typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void outlineQuad(const point4_t Area, const CB Callback)
    {
        if (Currentline == Area.y0 || Currentline == Area.y1 - 1)
            return Callback({ Area.x0, Currentline - Area.y0 }, Area.x1 - Area.x0 + 1);

        if (Currentline > Area.y0 && Currentline < Area.y1)
        {
            Callback({ Area.x0, Currentline - Area.y0 }, 1);
            Callback({ Area.x1, Currentline - Area.y0 }, 1);
        }
    }

    // Outline and fill circles.
    template <bool Fill, typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void drawCircle(const point2_t Position, const float Radius, const CB Callback)
    {
        const int16_t Top{ Position.y - int16_t(Radius) };
        const double R2{ Radius * Radius };
        int16_t X{}, Y{ int16_t(Radius) };

        // Helpers to keep the algorithm cleaner.
        auto doCallback = [&Callback, Top](const point2_t Position, const size_t Length = 1) -> void
        {
            if (Position.y == Currentline)
            {
                const point2_t Line{ Position.x, std::min(int16_t(Position.x + Length), int16_t(Scanlinelength)) };
                Callback({ Line.x, Currentline - Top }, std::max(int16_t(Line.y - Line.x), int16_t(1)));
            }
        };
        auto doDrawing = [&](const point2_t Origin, const point2_t Size) -> void
        {
            if (Size.x == 0)
            {
                doCallback({ Origin.x, int16_t(Origin.y + Size.y) });
                doCallback({ Origin.x, int16_t(Origin.y - Size.y) });

                if (Fill)
                {
                    doCallback({ int16_t(Origin.x - Size.y), Origin.y }, Size.y * 2);
                }
                else
                {
                    doCallback({ int16_t(Origin.x + Size.y), Origin.y });
                    doCallback({ int16_t(Origin.x - Size.y), Origin.y });
                }
                return;
            }

            if (Size.x == Size.y)
            {
                if (Fill)
                {
                    doCallback({ int16_t(Origin.x - Size.x), int16_t(Origin.y + Size.y) }, Size.x * 2);
                    doCallback({ int16_t(Origin.x - Size.x), int16_t(Origin.y - Size.y) }, Size.x * 2);
                }
                else
                {
                    doCallback({ int16_t(Origin.x + Size.x), int16_t(Origin.y + Size.y) });
                    doCallback({ int16_t(Origin.x - Size.x), int16_t(Origin.y + Size.y) });
                    doCallback({ int16_t(Origin.x + Size.x), int16_t(Origin.y - Size.y) });
                    doCallback({ int16_t(Origin.x - Size.x), int16_t(Origin.y - Size.y) });
                }
                return;
            }

            if (Size.x < Size.y)
            {
                if (Fill)
                {
                    /*
                        HACK(Convery):
                        We can not draw multiple lines with the same Y coord
                        because it messes up the alpha-blending and creates
                        an orb-like effect. https://i.imgur.com/AhdOsqp.png
                    */
                    thread_local int16_t PreviousY = -1;
                    if (PreviousY != Size.y)
                    {
                        doCallback({ int16_t(Origin.x - Size.x), int16_t(Origin.y + Size.y) }, Size.x * 2);
                        doCallback({ int16_t(Origin.x - Size.x), int16_t(Origin.y - Size.y) }, Size.x * 2);
                        PreviousY = Size.y;
                    }
                    doCallback({ int16_t(Origin.x - Size.y), int16_t(Origin.y + Size.x) }, Size.y * 2);
                    doCallback({ int16_t(Origin.x - Size.y), int16_t(Origin.y - Size.x) }, Size.y * 2);
                }
                else
                {
                    doCallback({ int16_t(Origin.x + Size.x), int16_t(Origin.y + Size.y) });
                    doCallback({ int16_t(Origin.x - Size.x), int16_t(Origin.y + Size.y) });
                    doCallback({ int16_t(Origin.x + Size.x), int16_t(Origin.y - Size.y) });
                    doCallback({ int16_t(Origin.x - Size.x), int16_t(Origin.y - Size.y) });
                    doCallback({ int16_t(Origin.x + Size.y), int16_t(Origin.y + Size.x) });
                    doCallback({ int16_t(Origin.x - Size.y), int16_t(Origin.y + Size.x) });
                    doCallback({ int16_t(Origin.x + Size.y), int16_t(Origin.y - Size.x) });
                    doCallback({ int16_t(Origin.x - Size.y), int16_t(Origin.y - Size.x) });
                }
                return;
            }
        };

        // Draw the cardinals..
        doDrawing(Position, { X, Y });

        // Draw the rest of the owl..
        Y = int16_t(std::sqrt(R2 - int16_t(1)) + 0.5f); X++;
        while (X < Y)
        {
            doDrawing(Position, { X++, Y });
            Y = int16_t(std::sqrt(R2 - X * X) + 0.5f);
        }

        // Precision..
        if (X == Y)
        {
            doDrawing(Position, { X, Y });
        }
    }
    template<typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void fillCircle(const point2_t Position, const float Radius, const CB Callback)
    {
        return drawCircle<true>(Position, Radius, Callback);
    }
    template<typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void outlineCircle(const point2_t Position, const float Radius, const CB Callback)
    {
        return drawCircle<false>(Position, Radius, Callback);
    }
}

namespace Engine::Rendering::Draw
{
    template <bool Outline> void Quad(const texture_t Color, const point4_t Area)
    {
        BitBlt(Area, Color);
    }
    template <bool Outline> void Quad(const rgba_t Color, const point4_t Area)
    {
        if (Outline)
        {
            BitBlt({ Area.x0, Area.y0, Area.x1, Area.y0 + 1 }, Color);
            BitBlt({ Area.x0, Area.y1 - 1, Area.x1, Area.y1 }, Color);
            BitBlt({ Area.x0, Area.y0, Area.x0 + 1, Area.y1 }, Color);
            BitBlt({ Area.x1 - 1, Area.y0, Area.x1, Area.y1 }, Color);
        }
        else
        {
            BitBlt(Area, Color);
        }
    }


    template <bool Outline> void Circle(const texture_t Color, const point2_t Position, const float Radius)
    {
        const auto Lambda = [&](const point2_t Position, const int16_t Length) -> void
        {
            for (int16_t i = 0; i < Length; ++i)
            {
                const auto Index{ (Position.y % Color.Dimensions.y) * Color.Dimensions.x + i };

                if (Color.Pixelsize == sizeof(pixel24_t)) setPixel(Position.x + i, ((pixel24_t *)Color.Data)[Index]);
                else setPixel(Position.x + i, ((pixel32_t *)Color.Data)[Index]);
            }
        };

        if (!Outline) Internal::fillCircle(Position, Radius, Lambda);
        else Internal::outlineCircle(Position, Radius, Lambda);
    }
    template <bool Outline> void Circle(const rgba_t Color, const point2_t Position, const float Radius)
    {
        auto Pixel{ fromRGBA(Color) };
        const auto Lambda = [&](const point2_t Position, const int16_t Length) -> void
        {
            for (int16_t i = 0; i < Length; ++i)
            {
                setPixel(Position.x + i, Pixel);
            }
        };
        if (!Outline)
        {
            Internal::fillCircle(Position, Radius, Lambda);
            Pixel.Raw[3] /= 2;
            Internal::outlineCircle(Position, Radius, Lambda);
        }
        else Internal::outlineCircle(Position, Radius, Lambda);
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
    Draw::Circle<false>(texture_t(), point2_t(), float());
    Draw::Circle<true>(texture_t(), point2_t(), float());
    Draw::Circle<false>(rgba_t(), point2_t(), float());
    Draw::Circle<true>(rgba_t(), point2_t(), float());
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
