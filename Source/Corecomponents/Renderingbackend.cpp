/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides simple rendering using the subsystem available.
*/

#include "../Stdinclude.hpp"
#include <queue>

extern float gWidth, gHeight, gPosX, gPosY;
static float lWidth{}, lHeight{};

#if defined(_WIN32)

namespace Rendering
{
    static std::queue<vec4_t> Invalidareas{};
    static HDC Surfacecontext{};
    static pixel24_t *Canvas{};
    static HBITMAP Surface{};

    // System-code interaction, assumes single-threaded sync.
    void onPresent(const void *Handle)
    {
        StretchBlt((HDC)Handle, 0, 0, gWidth, gHeight, Surfacecontext, 0, 0, Resolution.x, Resolution.y, SRCCOPY);
        //BitBlt((HDC)Handle, 0, 0, gWidth, gHeight, Surfacecontext, 0, 0, SRCCOPY);
    }
    void onRender()
    {
        if (!Canvas)
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
            Surface = CreateDIBSection(Devicecontext, &Format, DIB_RGB_COLORS, (void **)&Canvas, NULL, 0);
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
        std::function<void(Element_t *, vec4_t Clip)> Lambda = [&](Element_t *Parent, vec4_t Clip) -> void
        {
            Parent->onRender(Parent, Clip);
            for (const auto &Child : Parent->Children) Lambda(Child, Clip);
        };

        // Update all areas.
        while (!Invalidareas.empty())
        {
            Lambda(getRootelement(), Invalidareas.front());
            Invalidareas.pop();
        }
    }

    // User-code interaction.
    namespace Draw
    {
        // Windows wants to use BGR.
        ainline pixel24_t fromRGBA(const rgba_t Color)
        {
            return
            {
                uint8_t(Color.B <= 1 ? Color.B * 255 : Color.B),
                uint8_t(Color.G <= 1 ? Color.G * 255 : Color.G),
                uint8_t(Color.R <= 1 ? Color.R * 255 : Color.R)
            };
        }
        ainline pixel24_t Blend(const pixel24_t Base, const pixel24_t Overlay, const double Alpha)
        {
            return
            {
                uint8_t(Base.B * (1.0 - Alpha) + Overlay.B * Alpha),
                uint8_t(Base.G * (1.0 - Alpha) + Overlay.G * Alpha),
                uint8_t(Base.R * (1.0 - Alpha) + Overlay.R * Alpha)
            };
        }
        ainline void Setpixel(const pixel24_t Input, const size_t X, const size_t Y, const double Alpha)
        {
            Canvas[Y * size_t(Resolution.x) + X] = Blend(Canvas[Y * size_t(Resolution.x - 1) + X], Input, Alpha);
        }
        ainline void Blendedfill(const pixel24_t Input, const vec4_t Box, const double Alpha)
        {
            for (int64_t Y = std::clamp(Box.y0, -1.0f, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0f, Resolution.y - 1); ++Y)
            {
                for (int64_t X = std::clamp(Box.x0, -1.0f, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0f, Resolution.x - 1); ++X)
                {
                    if (X >= 0 && Y >= 0) Canvas[Y * size_t(Resolution.x) + X] = Blend(Canvas[Y * size_t(Resolution.x) + X], Input, Alpha);
                }
            }
        }
        ainline void Setpixel(const pixel24_t Input, const size_t X, const size_t Y)
        {
            Canvas[Y * size_t(Resolution.x) + X] = Input;
        }
        ainline void Solidfill(const pixel24_t Input, const vec4_t Box)
        {
            for (int64_t Y = std::clamp(Box.y0, -1.0f, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0f, Resolution.y - 1); ++Y)
            {
                for (int64_t X = std::clamp(Box.x0, -1.0f, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0f, Resolution.x - 1); ++X)
                {
                    if (X >= 0 && Y >= 0) Canvas[Y * size_t(Resolution.x) + X] = Input;
                }
            }
        }

        // Basic drawing.
        void Quad(const rgba_t Color, const vec4_t Box, const vec4_t Clip)
        {
            // Invisible element.
            if (Color.A == 0.0) return;

            // Clipped area.
            const vec4_t Area
            {
                std::max(Box.x0, Clip.x0), std::max(Box.y0, Clip.y0),
                std::min(Box.x1, Clip.x1), std::min(Box.y1, Clip.y1)
            };

            // Solid element.
            if (Color.A == 1.0) Solidfill(fromRGBA(Color), Area);

            // Transparent element.
            else return Blendedfill(fromRGBA(Color), Area, Color.A);
        }
        void Line(const rgba_t Color, const vec4_t Box, const vec4_t Clip)
        {
            // Invisible element.
            if (Color.A == 0.0) return;

            // Clipped area.
            vec4_t Area{ Box };

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
                        if (Steep) Setpixel(Pixel, Y, X, Color.A);
                        else Setpixel(Pixel, X, Y, Color.A);
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
        void Border(const rgba_t Color, const vec4_t Box, const vec4_t Clip)
        {
            Line(Color, { Box.x0, Box.y0, Box.x1, Box.y0 }, Clip);
            Line(Color, { Box.x0, Box.y1, Box.x1, Box.y1 }, Clip);

            Line(Color, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 }, Clip);
            Line(Color, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 }, Clip);
        }

        // Textured drawing.
        void Texturedquad(const texture_t Texture, const vec4_t Box, const vec4_t Clip)
        {
            const auto Colors{ Texture::getGradient(Texture) }; if (!Colors) return;
            const size_t Colorcount{ Colors->size() };
            size_t Colorindex{};

            // Fill the quad.
            for (int64_t Y = std::clamp(Box.y0, -1.0f, Resolution.y - 1); Y <= std::clamp(Box.y1, -1.0f, Resolution.y - 1); ++Y)
            {
                for (int64_t X = std::clamp(Box.x0, -1.0f, Resolution.x - 1); X <= std::clamp(Box.x1, -1.0f, Resolution.x - 1); ++X)
                {
                    // Only draw inside the canvas.
                    if (X >= 0 && Y >= 0)
                    {
                        // Only draw inside the clipped area.
                        if (X >= Clip.x0 && X <= Clip.x1 && Y >= Clip.y0 && Y <= Clip.y1)
                        {
                            Setpixel((*Colors)[Colorindex++ % Colorcount], X, Y);
                        }
                    }
                }
            }
        }
        void Texturedline(const texture_t Texture, const vec4_t Box, const vec4_t Clip)
        {
            // Clipped area.
            vec4_t Area{ Box };

            // Check if the line is steep and invert.
            const bool Steep{ std::abs(Area.x0 - Area.x1) < std::abs(Area.y0 - Area.y1) };
            if (Area.x0 > Area.x1) std::swap(Area.x0, Area.x1);
            if (Area.y0 > Area.y1) std::swap(Area.y0, Area.y1);

            const auto DeltaX{ Area.x1 - Area.x0 };
            const auto DeltaY{ Area.y1 - Area.y0 };

            // Straight line, just quad-fill.
            if (DeltaX == 0.0 || DeltaY == 0.0) return Texturedquad(Texture, Box, Clip);

            const auto Colors{ Texture::getGradient(Texture) }; if (!Colors) return;
            const size_t Colorcount{ Colors->size() };
            size_t Colorindex{};

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
                        if (Steep) Setpixel((*Colors)[Colorindex++ % Colorcount], Y, X);
                        else Setpixel((*Colors)[Colorindex++ % Colorcount], X, Y);
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
        void Texturedborder(const texture_t Texture, const vec4_t Box, const vec4_t Clip)
        {
            Texturedline(Texture, { Box.x0, Box.y0, Box.x1, Box.y0 }, Clip);
            Texturedline(Texture, { Box.x0, Box.y1, Box.x1, Box.y1 }, Clip);

            Texturedline(Texture, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 }, Clip);
            Texturedline(Texture, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 }, Clip);
        }
    }
    namespace Texture
    {
        std::atomic<uint32_t> Texturecount{ 0 };
        std::unordered_map<texture_t, std::vector<pixel24_t>> Texturemap;
        texture_t Creategradient(const size_t Steps, const rgba_t Color1, const rgba_t Color2)
        {
            auto TextureID{ Texturecount++ };
            auto Entry = &Texturemap[TextureID];
            Entry->reserve(Steps);

            // Normalize the colors.
            const rgba_t ColorA{ Color1.R <= 1 ? Color1.R * 255 : Color1.R, Color1.G <= 1 ? Color1.G * 255 : Color1.G, Color1.B <= 1 ? Color1.B * 255 : Color1.B };
            const rgba_t ColorB{ Color2.R <= 1 ? Color2.R * 255 : Color2.R, Color2.G <= 1 ? Color2.G * 255 : Color2.G, Color2.B <= 1 ? Color2.B * 255 : Color2.B };

            // Generate half the steps from each direction.
            for (float i = 0; i < 1; i += (1.0 / (Steps / 2)))
            {
                rgba_t Blended;
                Blended.R = (ColorA.R / 255 * i) + (ColorB.R / 255 * (1 - i));
                Blended.G = (ColorA.G / 255 * i) + (ColorB.G / 255 * (1 - i));
                Blended.B = (ColorA.B / 255 * i) + (ColorB.B / 255 * (1 - i));
                Blended.A = 1;

                Entry->push_back(Draw::fromRGBA(Blended));
            }
            for (float i = 0; i < 1; i += (1.0 / (Steps / 2)))
            {
                rgba_t Blended;
                Blended.R = (ColorB.R / 255 * i) + (ColorA.R / 255 * (1 - i));
                Blended.G = (ColorB.G / 255 * i) + (ColorA.G / 255 * (1 - i));
                Blended.B = (ColorB.B / 255 * i) + (ColorA.B / 255 * (1 - i));
                Blended.A = 1;

                Entry->push_back(Draw::fromRGBA(Blended));
            }

            return TextureID;
        }
        std::vector<pixel24_t> *getGradient(const texture_t ID)
        {
            if (const auto Item = Texturemap.find(ID); Item != Texturemap.end())
                return &Item->second;
            return nullptr;
        }
    }

    void Invalidatearea(const vec4_t Box)
    {
        Invalidareas.push(Box);
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
        void Quad(rgba_t Color, vec4_t Box, vec4_t Margin) {}
        void Line(rgba_t Color, vec4_t Box, vec4_t Margin) {}
        void Border(rgba_t Color, vec4_t Box, vec4_t Margin) {}
    }
}

#endif
