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
    #pragma pack(1)
    struct Pixel_t
    {
        uint8_t R;
        uint8_t G;
        uint8_t B;
    };
    #pragma pack()

    constexpr vec2_t Resolution{ 1280, 720 };
    static HDC Surfacecontext{};
    static HBITMAP Surface{};
    static Pixel_t *Pixels{};

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
        // Windows wants to use RGB.
        ainline Pixel_t fromRGBA(const rgba_t Color)
        {
            return
            {
                uint8_t(Color.r <= 1 ? Color.r * 255 : Color.r),
                uint8_t(Color.g <= 1 ? Color.g * 255 : Color.g),
                uint8_t(Color.b <= 1 ? Color.b * 255 : Color.b)
            };
        }
        ainline Pixel_t Blend(const Pixel_t Base, const Pixel_t Overlay, const double Alpha)
        {
            return
            {
                uint8_t(Base.B * (1.0 - Alpha) + Overlay.B * Alpha),
                uint8_t(Base.G * (1.0 - Alpha) + Overlay.G * Alpha),
                uint8_t(Base.R * (1.0 - Alpha) + Overlay.R * Alpha)
            };
        }
        ainline void Setpixel(const Pixel_t Input, const size_t X, const size_t Y, const double Alpha)
        {
            Pixels[Y * size_t(Resolution.x) + X] = Blend(Pixels[Y * size_t(Resolution.x - 1) + X], Input, Alpha);
        }
        ainline void Blendedfill(const Pixel_t Input, const rect_t Box, const double Alpha)
        {
            for (size_t Y = std::clamp(Box.y0, -1.0, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0, Resolution.y - 1); ++Y)
            {
                for (size_t X = std::clamp(Box.x0, -1.0, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0, Resolution.x - 1); ++X)
                {
                    if (X > 0 && Y > 0) Pixels[Y * size_t(Resolution.x) + X] = Blend(Pixels[Y * size_t(Resolution.x) + X], Input, Alpha);
                }
            }
        }
        ainline void Setpixel(const Pixel_t Input, const size_t X, const size_t Y)
        {
            Pixels[Y * size_t(Resolution.x) + X] = Input;
        }
        ainline void Solidfill(const Pixel_t Input, const rect_t Box)
        {
            for (size_t Y = std::clamp(Box.y0, -1.0, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0, Resolution.y - 1); ++Y)
            {
                for (size_t X = std::clamp(Box.x0, -1.0, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0, Resolution.x - 1); ++X)
                {
                    if (X > 0 && Y > 0) Pixels[Y * size_t(Resolution.x) + X] = Input;
                }
            }
        }

        // Basic drawing.
        void Quad(const rgba_t Color, const rect_t Box)
        {
            if (Color.a == 0.0) return;
            if (Color.a == 1.0) Solidfill(fromRGBA(Color), Box);
            else return Blendedfill(fromRGBA(Color), Box, Color.a);
        }
        void Line(const rgba_t Color, const rect_t Box)
        {
            if (Color.a == 0.0) return;

            const auto DeltaX{ Box.x1 - Box.x0 };
            const auto DeltaY{ Box.y1 - Box.y0 };
            const auto Pixel{ fromRGBA(Color) };

            // Straight line.
            if (DeltaX == 0.0 || DeltaY == 0.0) return Solidfill(fromRGBA(Color), Box);

            // Slope down.
            if (DeltaX > DeltaY)
            {
                const auto K{ DeltaY / DeltaX };

                if (Color.a == 1.0)
                {
                    for (size_t X = std::clamp(Box.x0, -1.0, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0, Resolution.x - 1); ++X)
                    {
                        if (X > 0 && Box.y0 + (X - Box.x0) * K > 0) Setpixel(Pixel, X, Box.y0 + (X - Box.x0) * K);
                    }
                }
                else
                {
                    for (size_t X = std::clamp(Box.x0, -1.0, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0, Resolution.x - 1); ++X)
                    {
                        if (X > 0 && Box.y0 + (X - Box.x0) * K > 0) Setpixel(Pixel, X, Box.y0 + (X - Box.x0) * K, Color.a);
                    }
                }
            }
            else
            {
                const auto K{ DeltaX / DeltaY };

                if (Color.a == 1.0)
                {
                    for (size_t Y = std::clamp(Box.y0, -1.0, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0, Resolution.y - 1); ++Y)
                    {
                        if (Box.x0 + (Y - Box.x0) * K > 0 && Y > 0) Setpixel(Pixel, Box.x0 + (Y - Box.x0) * K, Y);
                    }
                }
                else
                {
                    for (size_t Y = std::clamp(Box.y0, -1.0, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0, Resolution.y - 1); ++Y)
                    {
                        if (Box.x0 + (Y - Box.x0) * K > 0 && Y > 0) Setpixel(Pixel, Box.x0 + (Y - Box.x0) * K, Y, Color.a);
                    }
                }
            }
        }
        void Border(const rgba_t Color, const rect_t Box)
        {
            Line(Color, { Box.x0, Box.y0, Box.x1, Box.y0 });
            Line(Color, { Box.x0, Box.y1, Box.x1, Box.y1 });

            Line(Color, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 });
            Line(Color, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 });
        }

        // Gradient drawing.
        void Quadgradient(const std::vector<rgba_t> Colors, const rect_t Box)
        {
            size_t Colorcount{ Colors.size() };
            std::vector<Pixel_t> Pixel;
            Pixel.reserve(Colorcount);
            size_t Colorindex{};

            // Use the Windows format of the pixels.
            for (const auto &Item : Colors) Pixel.push_back(fromRGBA(Item));

            for (size_t Y = std::clamp(Box.y0, -1.0, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0, Resolution.y - 1); ++Y)
            {
                for (size_t X = std::clamp(Box.x0, -1.0, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0, Resolution.x - 1); ++X)
                {
                    if (X > 0 && Y > 0) Setpixel(Pixel[Colorindex++ % Colorcount], X, Y);
                }
            }
        }
        void Linegradient(const std::vector<rgba_t> Colors, const rect_t Box)
        {
            const auto DeltaX{ Box.x1 - Box.x0 };
            const auto DeltaY{ Box.y1 - Box.y0 };

            // Straight line.
            if (DeltaX == 0.0 || DeltaY == 0.0) return Quadgradient(Colors, Box);

            size_t Colorcount{ Colors.size() };
            std::vector<Pixel_t> Pixel;
            Pixel.reserve(Colorcount);
            size_t Colorindex{};

            // Use the Windows format of the pixels.
            for (const auto &Item : Colors) Pixel.push_back(fromRGBA(Item));

            // Slope down.
            if (DeltaX > DeltaY)
            {
                const auto K{ DeltaY / DeltaX };

                for (size_t X = std::clamp(Box.x0, -1.0, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0, Resolution.x - 1); ++X)
                {
                    if (X > 0 && Box.y0 + (X - Box.x0) * K > 0) Setpixel(Pixel[Colorindex++ % Colorcount], X, Box.y0 + (X - Box.x0) * K);
                }
            }
            else
            {
                const auto K{ DeltaX / DeltaY };

                for (size_t Y = std::clamp(Box.y0, -1.0, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0, Resolution.y - 1); ++Y)
                {
                    if (Box.x0 + (Y - Box.x0) * K > 0 && Y > 0) Setpixel(Pixel[Colorindex++ % Colorcount], Box.x0 + (Y - Box.x0) * K, Y);
                }
            }
        }
        void Bordergradient(const std::vector<rgba_t> Colors, const rect_t Box)
        {
            Linegradient(Colors, { Box.x0, Box.y0, Box.x1, Box.y0 });
            Linegradient(Colors, { Box.x0, Box.y1, Box.x1, Box.y1 });

            Linegradient(Colors, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 });
            Linegradient(Colors, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 });
        }
    }

    std::vector<rgba_t> Creategradient(const size_t Steps, const rgba_t Color1, const rgba_t Color2)
    {
        std::vector<rgba_t> Colors;
        Colors.reserve(Steps);

        // Normalize the colors.
        const rgba_t ColorA{ Color1.b <= 1 ? Color1.b * 255 : Color1.b, Color1.g <= 1 ? Color1.g * 255 : Color1.g, Color1.r <= 1 ? Color1.r * 255 : Color1.r };
        const rgba_t ColorB{ Color2.b <= 1 ? Color2.b * 255 : Color2.b, Color2.g <= 1 ? Color2.g * 255 : Color2.g, Color2.r <= 1 ? Color2.r * 255 : Color2.r };

        // Generate half the steps from each direction.
        for (double i = 0; i < 1; i += (1.0 / (Steps / 2)))
        {
            rgba_t Blended;
            Blended.r = (ColorA.r / 255 * i) + (ColorB.r / 255 * (1 - i));
            Blended.g = (ColorA.g / 255 * i) + (ColorB.g / 255 * (1 - i));
            Blended.b = (ColorA.b / 255 * i) + (ColorB.b / 255 * (1 - i));
            Blended.a = 1;

            Colors.push_back(Blended);
        }
        for (double i = 0; i < 1; i += (1.0 / (Steps / 2)))
        {
            rgba_t Blended;
            Blended.r = (ColorB.r / 255 * i) + (ColorA.r / 255 * (1 - i));
            Blended.g = (ColorB.g / 255 * i) + (ColorA.g / 255 * (1 - i));
            Blended.b = (ColorB.b / 255 * i) + (ColorA.b / 255 * (1 - i));
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
