/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides simple rendering using the subsystem available.
*/

#include "../Stdinclude.hpp"

extern double gWidth, gHeight, gPosX, gPosY;
static double lWidth{}, lHeight{};

#if defined(_WIN32)

namespace Rendering
{
    static vec2_t Resolution{ 1280, 720 };
    static HDC Surfacecontext{};
    static HBITMAP Surface{};
    static uint8_t *Pixels{};

    // System-code interaction, assumes single-threaded sync.
    void onPresent(const void *Handle)
    {
        StretchBlt((HDC)Handle, 0, 0, gWidth, gHeight, Surfacecontext, 0, 0, Resolution.x, Resolution.y, SRCCOPY);
        //BitBlt((HDC)Handle, 0, 0, gWidth, gHeight, Surfacecontext, 0, 0, SRCCOPY);
    }
    vec2_t getResolution()
    {
        return Resolution;
    }
    void onRender()
    {
        if (!Pixels)
        {
            auto Devicecontext{ GetDC(NULL) };
            BITMAPINFO Format{};

            // Bitmap format.
            Format.bmiHeader.biSize = sizeof(BITMAPINFO);
            Format.bmiHeader.biHeight =  -Resolution.y;
            Format.bmiHeader.biWidth = Resolution.x;
            Format.bmiHeader.biCompression = BI_RGB;
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
        }

        // Update the elements if the resolution changed.
        if (lWidth != gWidth || lHeight != gHeight)
        {
            getRootelement()->Renderbox = { 0, 0, Resolution.x, Resolution.y };
            getRootelement()->Boundingbox = { 0, 0, gWidth, gHeight };
            getRootelement()->onModifiedstate(getRootelement());
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
            if (X >= Resolution.x || Y >= Resolution.y) return;
            std::memcpy(Pixels + Y * uint32_t(Resolution.x) * 3 + X * 3, &BGR, 3);
        }
        ainline void Setpixel(uint32_t X, uint32_t Y, rgba_t RGBA)
        {
            if (0.0 == RGBA.a) return;
            if (X >= Resolution.x || Y >= Resolution.y) return;
            if (1.0 == RGBA.a) return Setpixel(X, Y, toBGR(Normalize(RGBA)));

            uint32_t Base{ *(uint32_t *)(Pixels + Y * uint32_t(Resolution.x) * 3 + X * 3) };
            return Setpixel(X, Y, BlendBGR(Base, toBGR(Normalize(RGBA)), RGBA.a));
        }

        void Quad(rgba_t Color, rect_t Box)
        {
            for (uint32_t Y = std::max(Box.y0, 0.0); Y < std::min(Box.y1, Resolution.y); ++Y)
            {
                for (uint32_t X = std::max(Box.x0, 0.0); X < std::min(Box.x1, Resolution.x); ++X)
                {
                    Setpixel(X, Y, Color);
                }
            }
        }
        void Line(rgba_t Color, rect_t Box)
        {
            double DeltaX{ Box.x1 - Box.x0 };
            double DeltaY{ Box.y1 - Box.y0 };

            if (std::abs(DeltaX) > std::abs(DeltaY))
            {
                for (uint32_t x = std::max(std::min(Box.x0, Box.x1), 0.0); x <= std::min(std::max(Box.x0, Box.x1), Resolution.x); ++x)
                {
                    Setpixel(x, (Box.y0 + ((x - Box.x0) * (DeltaY / DeltaX))), Color);
                }
            }
            else
            {
                for (uint32_t y = std::max(std::min(Box.y0, Box.y1), 0.0); y <= std::min(std::max(Box.y0, Box.y1), Resolution.y); ++y)
                {
                    Setpixel((Box.x0 + ((y - Box.y0) * (DeltaX / DeltaY))), y, Color);
                }
            }
        }
        void Border(rgba_t Color, rect_t Box)
        {
            Line(Color, { Box.x0, Box.y0, Box.x1, Box.y0 });
            Line(Color, { Box.x0, Box.y1, Box.x1, Box.y1 });

            Line(Color, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 });
            Line(Color, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 });
        }

        void Quadgradient(std::vector<rgba_t> Colors, rect_t Box)
        {
            auto Colorsize{ Colors.size() };
            size_t Colorindex{};

            for (uint32_t Y = std::max(Box.y0, 0.0); Y < std::min(Box.y1, Resolution.y); ++Y)
            {
                for (uint32_t X = std::max(Box.x0, 0.0); X < std::min(Box.x1, Resolution.x); ++X)
                {
                    Setpixel(X, Y, Colors[Colorindex++ % Colorsize]);
                }
            }
        }
        void Linegradient(std::vector<rgba_t> Colors, rect_t Box)
        {
            double DeltaX{ Box.x1 - Box.x0 };
            double DeltaY{ Box.y1 - Box.y0 };
            auto Colorsize{ Colors.size() };
            size_t Colorindex{};

            if (std::abs(DeltaX) > std::abs(DeltaY))
            {
                for (uint32_t x = std::max(std::min(Box.x0, Box.x1), 0.0); x <= std::min(std::max(Box.x0, Box.x1), Resolution.x); ++x)
                {
                    Setpixel(x, (Box.y0 + ((x - Box.x0) * (DeltaY / DeltaX))), Colors[Colorindex++ % Colorsize]);
                }
            }
            else
            {
                for (uint32_t y = std::max(std::min(Box.y0, Box.y1), 0.0); y <= std::min(std::max(Box.y0, Box.y1), Resolution.y); ++y)
                {
                    Setpixel((Box.x0 + ((y - Box.y0) * (DeltaX / DeltaY))), y,  Colors[Colorindex++ % Colorsize]);
                }
            }
        }
        void Bordergradient(std::vector<rgba_t> Colors, rect_t Box)
        {
            Linegradient(Colors, { Box.x0, Box.y0, Box.x1, Box.y0 });
            Linegradient(Colors, { Box.x0, Box.y1, Box.x1, Box.y1 });

            Linegradient(Colors, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 });
            Linegradient(Colors, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 });
        }
    }

    std::vector<rgba_t> Creategradient(size_t Steps, rgba_t Color1, rgba_t Color2)
    {
        std::vector<rgba_t> Colors;
        Color1 = Draw::Normalize(Color1);
        Color2 = Draw::Normalize(Color2);

        for (double i = 0; i < 1; i += (1.0 / (Steps / 2)))
        {
            rgba_t Blended;
            Blended.r = (Color1.r / 255 * i) + (Color2.r / 255 * (1 - i));
            Blended.g = (Color1.g / 255 * i) + (Color2.g / 255 * (1 - i));
            Blended.b = (Color1.b / 255 * i) + (Color2.b / 255 * (1 - i));
            Blended.a = 1;

            Colors.push_back(Blended);
        }
        for (double i = 0; i < 1; i += (1.0 / (Steps / 2)))
        {
            rgba_t Blended;
            Blended.r = (Color2.r / 255 * i) + (Color1.r / 255 * (1 - i));
            Blended.g = (Color2.g / 255 * i) + (Color1.g / 255 * (1 - i));
            Blended.b = (Color2.b / 255 * i) + (Color1.b / 255 * (1 - i));
            Blended.a = 1;

            Colors.push_back(Blended);
        }

        return Colors;
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
