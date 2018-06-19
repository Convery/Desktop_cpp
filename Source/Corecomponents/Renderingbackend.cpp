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
        uint8_t B;
        uint8_t G;
        uint8_t R;
    };
    #pragma pack()


    static HDC Surfacecontext{};
    static HBITMAP Surface{};
    static Pixel_t *Pixels{};

    // System-code interaction, assumes single-threaded sync.
    void onPresent(const void *Handle)
    {
        StretchBlt((HDC)Handle, 0, 0, gWidth, gHeight, Surfacecontext, 0, 0, Resolution.x, Resolution.y, SRCCOPY);
        //BitBlt((HDC)Handle, 0, 0, gWidth, gHeight, Surfacecontext, 0, 0, SRCCOPY);
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
        std::function<void(Element_t *, rect_t Clip)> Lambda = [&](Element_t *Parent, rect_t Clip) -> void
        {
            Parent->onRender(Parent, Clip);
            for (const auto &Child : Parent->Children) Lambda(Child, Clip);
        };

        Lambda(getRootelement(), { -1, -1, Resolution.x, Resolution.y });
    }

    // User-code interaction.
    namespace Draw
    {
        // Windows wants to use BGR.
        ainline Pixel_t fromRGBA(const rgba_t Color)
        {
            return
            {
                uint8_t(Color.b <= 1 ? Color.b * 255 : Color.b),
                uint8_t(Color.g <= 1 ? Color.g * 255 : Color.g),
                uint8_t(Color.r <= 1 ? Color.r * 255 : Color.r)
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
            for (int64_t Y = std::clamp(Box.y0, -1.0, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0, Resolution.y - 1); ++Y)
            {
                for (int64_t X = std::clamp(Box.x0, -1.0, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0, Resolution.x - 1); ++X)
                {
                    if (X >= 0 && Y >= 0) Pixels[Y * size_t(Resolution.x) + X] = Blend(Pixels[Y * size_t(Resolution.x) + X], Input, Alpha);
                }
            }
        }
        ainline void Setpixel(const Pixel_t Input, const size_t X, const size_t Y)
        {
            Pixels[Y * size_t(Resolution.x) + X] = Input;
        }
        ainline void Solidfill(const Pixel_t Input, const rect_t Box)
        {
            for (int64_t Y = std::clamp(Box.y0, -1.0, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0, Resolution.y - 1); ++Y)
            {
                for (int64_t X = std::clamp(Box.x0, -1.0, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0, Resolution.x - 1); ++X)
                {
                    if (X >= 0 && Y >= 0) Pixels[Y * size_t(Resolution.x) + X] = Input;
                }
            }
        }

        // Basic drawing.
        void Quad(const rgba_t Color, const rect_t Box, const rect_t Clip)
        {
            // Invisible element.
            if (Color.a == 0.0) return;

            // Clipped area.
            const rect_t Area
            {
                std::max(Box.x0, Clip.x0), std::max(Box.y0, Clip.y0),
                std::min(Box.x1, Clip.x1), std::min(Box.y1, Clip.y1)
            };

            // Solid element.
            if (Color.a == 1.0) Solidfill(fromRGBA(Color), Area);

            // Transparent element.
            else return Blendedfill(fromRGBA(Color), Area, Color.a);
        }
        void Line(const rgba_t Color, const rect_t Box, const rect_t Clip)
        {
            // Invisible element.
            if (Color.a == 0.0) return;

            // Clipped area.
            rect_t Area{ Box };

            // Check if the line is steep and invert.
            const bool Steep{ std::abs(Area.x0 - Area.x1) < std::abs(Area.y0 - Area.y1) };
            if (Area.x0 > Area.x1)
            {
                std::swap(Area.x0, Area.x1);
                std::swap(Area.y0, Area.y1);
            }

            const auto DeltaX{ Area.x1 - Area.x0 };
            const auto DeltaY{ Area.y1 - Area.y0 };

            // Straight line, just quad-fill.
            if (DeltaX == 0.0 || DeltaY == 0.0) return Quad(Color, Area);

            // Draw the actual line.
            const auto Pixel = fromRGBA(Color);
            int64_t Y{ int64_t(Area.y0) }, Error{};
            const int64_t Deltaerror{ int64_t(std::abs(DeltaY) * 2) };
            for (int64_t X = Area.x0; X <= Area.x1; ++X)
            {
                // Only draw inside the canvas.
                if (X >= 0 && Y >= 0)
                {
                    // Only draw inside the clipped area.
                    if (X >= Clip.x0 && X <= Clip.x1 && Y >= Clip.y0 && Y <= Clip.y1)
                    {
                        // Invert the coordinates if too steep.
                        if (Steep) Setpixel(Pixel, Y, X, Color.a);
                        else Setpixel(Pixel, X, Y, Color.a);
                    }
                }

                // Update the error offset.
                Error += Deltaerror;
                if (Error > DeltaX)
                {
                    Y += Area.y1 > Area.y0 ? 1 : -1;
                    Error -= DeltaX * 2;
                }
            }
        }
        void Border(const rgba_t Color, const rect_t Box, const rect_t Clip)
        {
            Line(Color, { Box.x0, Box.y0, Box.x1, Box.y0 }, Clip);
            Line(Color, { Box.x0, Box.y1, Box.x1, Box.y1 }, Clip);

            Line(Color, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 }, Clip);
            Line(Color, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 }, Clip);
        }

        // Gradient drawing.
        void Quadgradient(const std::vector<rgba_t> Colors, const rect_t Box, const rect_t Clip)
        {
            const size_t Colorcount{ Colors.size() };
            std::vector<Pixel_t> Pixel;
            Pixel.reserve(Colorcount);
            size_t Colorindex{};

            // Use the Windows format of the pixels.
            for (const auto &Item : Colors) Pixel.push_back(fromRGBA(Item));

            // Fill the quad.
            for (int64_t Y = std::clamp(Box.y0, -1.0, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0, Resolution.y - 1); ++Y)
            {
                for (int64_t X = std::clamp(Box.x0, -1.0, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0, Resolution.x - 1); ++X)
                {
                    // Only draw inside the canvas.
                    if (X >= 0 && Y >= 0)
                    {
                        // Only draw inside the clipped area.
                        if (X >= Clip.x0 && X <= Clip.x1 && Y >= Clip.y0 && Y <= Clip.y1)
                        {
                            Setpixel(Pixel[Colorindex++ % Colorcount], X, Y);
                        }
                    }
                }
            }
        }
        void Linegradient(const std::vector<rgba_t> Colors, const rect_t Box, const rect_t Clip)
        {
            // Clipped area.
            rect_t Area{ Box };

            // Check if the line is steep and invert.
            const bool Steep{ std::abs(Area.x0 - Area.x1) < std::abs(Area.y0 - Area.y1) };
            if (Area.x0 > Area.x1) std::swap(Area.x0, Area.x1);
            if (Area.y0 > Area.y1) std::swap(Area.y0, Area.y1);

            const auto DeltaX{ Area.x1 - Area.x0 };
            const auto DeltaY{ Area.y1 - Area.y0 };

            // Straight line, just quad-fill.
            if (DeltaX == 0.0 || DeltaY == 0.0) return Quadgradient(Colors, Box, Clip);

            const size_t Colorcount{ Colors.size() };
            std::vector<Pixel_t> Pixel;
            Pixel.reserve(Colorcount);
            size_t Colorindex{};

            // Use the Windows format of the pixels.
            for (const auto &Item : Colors) Pixel.push_back(fromRGBA(Item));

            // Draw the actual line.
            int64_t Y{ int64_t(Area.y0) }, Error{};
            const int64_t Deltaerror{ int64_t(std::abs(DeltaY) * 2) };
            for (int64_t X = Area.x0; X <= Area.x1; ++X)
            {
                // Only draw inside the canvas.
                if (X >= 0 && Y >= 0)
                {
                    // Only draw inside the clipped area.
                    if (X >= Clip.x0 && X <= Clip.x1 && Y >= Clip.y0 && Y <= Clip.y1)
                    {
                        // Invert the coordinates if too steep.
                        if (Steep) Setpixel(Pixel[Colorindex++ % Colorcount], Y, X);
                        else Setpixel(Pixel[Colorindex++ % Colorcount], X, Y);
                    }
                }

                // Update the error offset.
                Error += Deltaerror;
                if (Error > DeltaX)
                {
                    Y += Area.y1 > Area.y0 ? 1 : -1;
                    Error -= DeltaX * 2;
                }
            }
        }
        void Bordergradient(const std::vector<rgba_t> Colors, const rect_t Box, const rect_t Clip)
        {
            Linegradient(Colors, { Box.x0, Box.y0, Box.x1, Box.y0 }, Clip);
            Linegradient(Colors, { Box.x0, Box.y1, Box.x1, Box.y1 }, Clip);

            Linegradient(Colors, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 }, Clip);
            Linegradient(Colors, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 }, Clip);
        }
    }

    std::vector<rgba_t> Creategradient(const size_t Steps, const rgba_t Color1, const rgba_t Color2)
    {
        std::vector<rgba_t> Colors;
        Colors.reserve(Steps);

        // Normalize the colors.
        const rgba_t ColorA{ Color1.r <= 1 ? Color1.r * 255 : Color1.r, Color1.g <= 1 ? Color1.g * 255 : Color1.g, Color1.b <= 1 ? Color1.b * 255 : Color1.b };
        const rgba_t ColorB{ Color2.r <= 1 ? Color2.r * 255 : Color2.r, Color2.g <= 1 ? Color2.g * 255 : Color2.g, Color2.b <= 1 ? Color2.b * 255 : Color2.b };

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
