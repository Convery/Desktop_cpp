/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides simple rendering using the subsystem available.
*/

#include "../Stdinclude.hpp"

extern double gWidth, gHeight, gPosX, gPosY;

#if defined(_WIN32)

namespace Rendering
{
    static HDC Surfacecontext{};
    static HBITMAP Surface{};
    static uint8_t *Pixels{};

    // System-code interaction, assumes single-threaded sync.
    void onPresent(const void *Handle)
    {
        BitBlt((HDC)Handle, 0, 0, gWidth, gHeight, Surfacecontext, 0, 0, SRCCOPY);
    }
    void onRender()
    {
        static double lWidth{ gWidth }, lHeight{ gHeight };
        if (!Pixels || lWidth != gWidth || lHeight != gHeight)
        {
            auto Devicecontext{ GetDC(NULL) };
            BITMAPINFO Format{};

            // Bitmap format.
            Format.bmiHeader.biSize = sizeof(BITMAPINFO);
            Format.bmiHeader.biCompression = BI_RGB;
            Format.bmiHeader.biHeight =  -gHeight;
            Format.bmiHeader.biWidth = gWidth;
            Format.bmiHeader.biBitCount = 24;
            Format.bmiHeader.biPlanes = 1;

            // Cleanup any allocated memory.
            if (Surface) DeleteObject(Surface);
            if (Surfacecontext) DeleteDC(Surfacecontext);

            // Create the new surface.
            Surface = CreateDIBSection(Devicecontext, &Format, DIB_RGB_COLORS, (void **)&Pixels, NULL, 0);
            Surfacecontext = CreateCompatibleDC(Devicecontext);
            SelectObject(Surfacecontext, Surface);

            // C-style cleanup needed.
            DeleteDC(Devicecontext);

            // Locals update.
            lWidth = gWidth; lHeight = gHeight;
        }

        // Render the element-tree.
        std::function<void(Element_t *)> Lambda = [&](Element_t *Parent) -> void
        {
            Parent->onRender(Parent);
            for (const auto &Child : Parent->Children) Lambda(Child);
        };

        Lambda(getRootelement());
    }

    // User-code interaction.
    namespace Draw
    {
        // Windows wants to use BGR.
        ainline uint32_t toBGR(rgba_t Color)
        {
            return uint32_t
            {
                uint32_t(Color.r) << 16 |
                uint32_t(Color.g) << 8 |
                uint32_t(Color.b)
            };
        }
        ainline rgba_t Normalize(rgba_t Input)
        {
            if (Input.r <= 1) Input.r *= 255;
            if (Input.g <= 1) Input.g *= 255;
            if (Input.b <= 1) Input.b *= 255;
            Input.a = std::clamp(Input.a, 0.0, 1.0);
            return Input;
        }
        ainline uint32_t BlendBGR(uint32_t Base, uint32_t Input, double Alpha)
        {
            uint32_t Blended{};
            ((uint8_t *)&Blended)[0] = (((uint8_t *)&Base)[0] * (1.0f - Alpha)) + ((uint8_t *)&Input)[0] * Alpha;
            ((uint8_t *)&Blended)[1] = (((uint8_t *)&Base)[1] * (1.0f - Alpha)) + ((uint8_t *)&Input)[1] * Alpha;
            ((uint8_t *)&Blended)[2] = (((uint8_t *)&Base)[2] * (1.0f - Alpha)) + ((uint8_t *)&Input)[2] * Alpha;
            return Blended;
        }
        ainline void Setpixel(uint32_t X, uint32_t Y, uint32_t BGR)
        {
            if (X >= gWidth || Y >= gHeight) return;
            std::memcpy(Pixels + Y * uint32_t(gWidth) * 3 + X * 3, &BGR, 3);
        }
        ainline void Setpixel(uint32_t X, uint32_t Y, rgba_t RGBA)
        {
            if (0.0 == RGBA.a) return;
            if (X >= gWidth || Y >= gHeight) return;
            if (1.0 == RGBA.a) return Setpixel(X, Y, toBGR(Normalize(RGBA)));

            uint32_t Base{ *(uint32_t *)(Pixels + Y * uint32_t(gWidth) * 3 + X * 3) };
            return Setpixel(X, Y, BlendBGR(Base, toBGR(Normalize(RGBA)), RGBA.a));
        }

        void Quad(rgba_t Color, rect_t Box)
        {
            uint32_t Localcolor = toBGR(Normalize(Color));
            for (uint32_t Y = std::max(Box.y0, 0.0); Y < std::min(Box.y1, gHeight); ++Y)
            {
                for (uint32_t X = std::max(Box.x0, 0.0); X < std::min(Box.x1, gWidth); ++X)
                {
                    Setpixel(X, Y, Localcolor);
                }
            }
        }
        void Line(rgba_t Color, rect_t Box)
        {
            double DeltaX{ Box.x1 - Box.x0 };
            double DeltaY{ Box.y1 - Box.y0 };

            if (std::abs(DeltaX) > std::abs(DeltaY))
            {
                for (uint32_t x = std::max(std::min(Box.x0, Box.x1), 0.0); x <= std::min(std::max(Box.x0, Box.x1), gWidth); ++x)
                {
                    Setpixel(x, (Box.y0 + ((x - Box.x0) * (DeltaY / DeltaX))), Color);
                }
            }
            else
            {
                for (uint32_t y = std::max(std::min(Box.y0, Box.y1), 0.0); y <= std::min(std::max(Box.y0, Box.y1), gHeight); ++y)
                {
                    Setpixel((Box.x0 + ((y - Box.y0) * (DeltaX / DeltaY))), y, Color);
                }
            }
        }
        void Border(rgba_t Color, rect_t Box)
        {
            Line(Color, { Box.x0, Box.y0, Box.x1, Box.y0 });
            Line(Color, { Box.x0, Box.y1, Box.x1, Box.y1 });

            Line(Color, { Box.x0, Box.y0, Box.x0, Box.y1 });
            Line(Color, { Box.x1, Box.y0, Box.x1, Box.y1 });
        }
    }
}

#else

namespace Rendering
{
    // System-code interaction, assumes single-threaded sync.
    void onInit(double Width, double Height) {}
    void onPresent(const void *Handle) {}
    void onRender() {}

    // User-code interaction.
    namespace Draw
    {
        void Quad(rgba_t Color, rect_t Box, rect_t Margin) {}
        void Line(rgba_t Color, rect_t Box, rect_t Margin) {}
        void Border(rgba_t Color, rect_t Box, rect_t Margin) {}
    }
}

#endif
