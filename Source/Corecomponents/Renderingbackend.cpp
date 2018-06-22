/*
    Initial author: Convery (tcn@ayria.se)
    Started: 17-06-2018
    License: MIT

    Provides simple rendering using the subsystem available.
*/

#include "../Stdinclude.hpp"
#include <queue>

extern vec2_t Windowdimensions;

#if defined(_WIN32)

namespace Rendering
{
    std::queue<vec4_t> Dirtyareas{};
    vec4_t Clippingarea{};
    HDC Surfacecontext{};
    Pixel_t *Canvas{};
    bool Nativeres{};

    // Create our canvas for future drawing.
    void Createcanvas(const vec2_t lResolution)
    {
        static HBITMAP Surface{};

        auto Devicecontext{ GetDC(NULL) };
        BITMAPINFO Format{};

        // Bitmap format, upside-down because Windows.
        Format.bmiHeader.biSize = sizeof(BITMAPINFO);
        Format.bmiHeader.biHeight = -lResolution.y;
        Format.bmiHeader.biWidth = lResolution.x;
        Format.bmiHeader.biCompression = BI_RGB;
        Format.bmiHeader.biBitCount = 24;
        Format.bmiHeader.biPlanes = 1;

        // Cleanup any old memory.
        if (Surface) DeleteObject(Surface);
        if (Surfacecontext) DeleteDC(Surfacecontext);

        // Create the new surface.
        Surface = CreateDIBSection(Devicecontext, &Format, DIB_RGB_COLORS, (void **)&Canvas, NULL, 0);
        Surfacecontext = CreateCompatibleDC(Devicecontext);
        SelectObject(Surfacecontext, Surface);

        // C-style cleanup needed.
        DeleteDC(Devicecontext);
    }

    // Internal state-access.
    void Invalidatearea(const vec4_t Area)
    {
        Dirtyareas.push(Area);
    }
    void setClipping(const vec4_t Area)
    {
        Clippingarea = Area;
    }
    void onPresent(const void *Handle)
    {
        if (Nativeres) BitBlt((HDC)Handle, 0, 0,  Windowdimensions.x,  Windowdimensions.y, Surfacecontext, 0, 0, SRCCOPY);
        else StretchBlt((HDC)Handle, 0, 0,  Windowdimensions.x,  Windowdimensions.y, Surfacecontext, 0, 0, Resolution.x, Resolution.y, SRCCOPY);
    }
    void onRender()
    {
        // Create the canvas if needed, one row larger than needed because perf > mem.
        if (!Canvas) Createcanvas({ Resolution.x, Resolution.y + 1 });

        // Update the root element if the window changed.
        static vec2_t lDimensions{};
        if (lDimensions.x != Windowdimensions.x || lDimensions.y != Windowdimensions.y)
        {
            Scene::getRootelement()->Boundingbox = { 0, 0, Windowdimensions.x, Windowdimensions.y };
            Scene::getRootelement()->Renderbox = { 0, 0, Resolution.x, Resolution.y };
            Scene::getRootelement()->onModifiedstate(Scene::getRootelement());
            lDimensions = Windowdimensions;

            Nativeres = Windowdimensions.x == Resolution.x && Windowdimensions.y == Resolution.y;
        }

        // Render the element-tree.
        std::function<void(Element_t *)> doRendering = [&](Element_t *Parent) -> void
        {
            Parent->onRender(Parent);
            for (const auto &Child : Parent->Children) doRendering(Child);
        };

        // Re-draw the dirty areas.
        while (!Dirtyareas.empty())
        {
            setClipping(Dirtyareas.front());
            Dirtyareas.pop();

            doRendering(Scene::getRootelement());
        }
    }

    // Internal drawing, no boundary-checking.
    namespace Internal
    {
        // Windows wants to use BGR colors.
        void setPixel(const size_t X, const size_t Y, const Pixel_t Pixel, const float Alpha)
        {
            setPixel(X, Y, blendPixel(Canvas[Y * size_t(Resolution.x) + X], Pixel, Alpha));
        }
        void fillPoly(const vec2_t *Vertices, const size_t Count, onDrawcallback Callback)
        {
            auto Nodes{ std::make_unique<int32_t[]>(Count + 1) };

            // For each row in the canvas.
            for (size_t Y = std::max(0.0f, Clippingarea.y0); Y <= std::min(Resolution.y, Clippingarea.y1); ++Y)
            {
                int32_t i{}, j{ int32_t(Count) - 1 }, Nodecount{};

                // Create a few nodes.
                for (i = 0; i < Count; ++i)
                {
                    if ((Vertices[i].y <= Y && Vertices[j].y >= Y) || (Vertices[j].y <= Y && Vertices[i].y >= Y))
                    {
                        if(Vertices[j].y - Vertices[i].y == 0) Nodes[Nodecount++] = Vertices[i].x + (Y - Vertices[i].y) * (Vertices[j].x - Vertices[i].x);
                        else Nodes[Nodecount++] = Vertices[i].x + ((Y - Vertices[i].y) / (Vertices[j].y - Vertices[i].y)) * (Vertices[j].x - Vertices[i].x);
                    }
                    j = i;
                }

                // Sort the nodes.
                i = 0;
                while (i < Nodecount - 1)
                {
                    if (Nodes[i] > Nodes[i + 1])
                    {
                        std::swap(Nodes[i], Nodes[i + 1]);
                        if (i) i--;
                    }
                    else i++;
                }

                // Fill the pixels between the nodes.
                for (i = 0; i < Nodecount; i += 2)
                {
                    if (Nodes[i] > std::min(Resolution.x, Clippingarea.x1)) break;
                    if (Nodes[i + 1] >= std::min(0.0f, Clippingarea.x0))
                    {
                        if (Nodes[i] < std::max(Clippingarea.x0, 0.0f)) Nodes[i] = std::max(Clippingarea.x0, 0.0f);
                        if (Nodes[i + 1] > std::min(Clippingarea.x1, Resolution.x - 1)) Nodes[i + 1] = std::min(Clippingarea.x1, Resolution.x - 1);

                        for (j = Nodes[i]; j <= Nodes[i + 1]; ++j)
                        {
                            Callback(j, Y);
                        }
                    }
                }
            }
        }
        Pixel_t blendPixel(const Pixel_t Base, const Pixel_t Overlay, const float Alpha)
        {
            return
            {
                uint8_t(Base.BGR.B * (1.0 - Alpha) + Overlay.BGR.B * Alpha),
                uint8_t(Base.BGR.G * (1.0 - Alpha) + Overlay.BGR.G * Alpha),
                uint8_t(Base.BGR.R * (1.0 - Alpha) + Overlay.BGR.R * Alpha)
            };
        }
        void setPixel(const size_t X, const size_t Y, const Pixel_t Pixel)
        {
            Canvas[Y * size_t(Resolution.x) + X] = Pixel;
        }
        Pixel_t fromRGBA(const rgba_t Color)
        {
            return
            {
                uint8_t(Color.B <= 1 ? Color.B * 255 : Color.B),
                uint8_t(Color.G <= 1 ? Color.G * 255 : Color.G),
                uint8_t(Color.R <= 1 ? Color.R * 255 : Color.R)
            };
        }
        vec4_t clipArea(const vec4_t Area)
        {
            return { std::max(Area.x0, Clippingarea.x0), std::max(Area.y0, Clippingarea.y0), std::min(Area.x1, Clippingarea.x1), std::min(Area.y1, Clippingarea.y1) };
        }
    }

    // Basic drawing, converts the color to internal format.
    namespace Soliddraw
    {
        template <> void Triangle<false>(const rgba_t Color, const vec2_t a, const vec2_t b, const vec2_t c)
        {
            if (Color.A == 0.0f) return;
            Line(Color, { a.x, a.y }, { b.x, b.y });
            Line(Color, { b.x, b.y }, { c.x, c.y });
            Line(Color, { c.x, c.y }, { a.x, a.y });
        }
        template <> void Triangle<true>(const rgba_t Color, const vec2_t a, const vec2_t b, const vec2_t c)
        {
            if (Color.A == 0.0f) return;
            vec2_t Vertices[]{ a, b, c };
            auto Pixel{ Internal::fromRGBA(Color) };

            if(Color.A == 1.0f) Internal::fillPoly(Vertices, 3, [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel); });
            else Internal::fillPoly(Vertices, 3, [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel, Color.A); });
        }
        template <> void Quad<false>(const rgba_t Color, const vec4_t Area)
        {
            if (Color.A == 0.0f) return;
            Line(Color, { Area.x0 + 1, Area.y0 }, { Area.x1 - 1, Area.y0 });
            Line(Color, { Area.x0 + 1, Area.y1 }, { Area.x1 - 1, Area.y1 });
            Line(Color, { Area.x0, Area.y0 }, { Area.x0, Area.y1 });
            Line(Color, { Area.x1, Area.y0 }, { Area.x1, Area.y1 });
        }
        template <> void Quad<true>(const rgba_t Color, const vec4_t Area)
        {
            if (Color.A == 0.0f) return;
            auto Pixel{ Internal::fromRGBA(Color) };
            vec2_t Vertices[]{ {Area.x0, Area.y0}, {Area.x1, Area.y0}, {Area.x1, Area.y1}, {Area.x0, Area.y1} };

            if (Color.A == 1.0f) Internal::fillPoly(Vertices, 4, [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel); });
            else Internal::fillPoly(Vertices, 4, [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel, Color.A); });
            Quad<false>(Color, Area);
        }
        void Line(const rgba_t Color, const vec2_t Start, const vec2_t Stop)
        {
            if (Color.A == 0.0f) return;
            vec2_t Vertices[]{ Start, Stop };
            auto Pixel{ Internal::fromRGBA(Color) };

            if(Color.A == 1.0f) Internal::fillPoly(Vertices, 2, [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel); });
            else Internal::fillPoly(Vertices, 2, [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel, Color.A); });
        }
        void Polygon(const rgba_t Color, const std::vector<vec2_t> Vertices)
        {
            if (Color.A == 0.0f) return;
            auto Pixel{ Internal::fromRGBA(Color) };

            if(Color.A == 1.0f) Internal::fillPoly(Vertices.data(), Vertices.size(), [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel); });
            else Internal::fillPoly(Vertices.data(), Vertices.size(), [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel, Color.A); });
        }
        void Circle(const rgba_t Color, const vec2_t Position, float Radius)
        {
            if (Color.A == 0.0f) return;
            const auto Clipped{ Internal::clipArea({ Position.x - Radius, Position.y - Radius, Position.x + Radius, Position.y + Radius}) };
            const int64_t Rad2{ int64_t(Radius * Radius) };
            const auto Pixel{ Internal::fromRGBA(Color) };
            const int64_t Diameter{ int64_t(Radius * 2) };

            for (int64_t i = 0; i < Rad2 << 2; ++i)
            {
                const int64_t tx{ int64_t((i % Diameter) - Radius) };
                const int64_t ty{ int64_t((i / Diameter) - Radius) };

                if (tx * tx + ty * ty <= Rad2)
                {
                    const int64_t X{ int64_t(Position.x) + tx };
                    const int64_t Y{ int64_t(Position.y) + ty };

                    if (X >= Clipped.x0 && X <= Clipped.x1 && Y >= Clipped.y0 && Y <= Clipped.y1)
                    {
                        if(Color.A == 1.0f) Internal::setPixel(X, Y, Pixel);
                        else Internal::setPixel(X, Y, Pixel, Color.A);
                    }
                }
            }
        }
    }
    namespace Textureddraw
    {
        template <> void Triangle<false>(const texture_t Color, const vec2_t a, const vec2_t b, const vec2_t c)
        {
            Line(Color, { a.x, a.y }, { b.x, b.y });
            Line(Color, { b.x, b.y }, { c.x, c.y });
            Line(Color, { c.x, c.y }, { a.x, a.y });
        }
        template <> void Triangle<true>(const texture_t Color, const vec2_t a, const vec2_t b, const vec2_t c)
        {
            vec2_t Vertices[]{ a, b, c };
            if(Color.Alpha == 1.0f) Internal::fillPoly(Vertices, 3, [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)]);
            });
            else Internal::fillPoly(Vertices, 3, [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)], Color.Alpha);
            });
        }
        template <> void Quad<false>(const texture_t Color, const vec4_t Area)
        {
            Line(Color, { Area.x0 + 1, Area.y0 }, { Area.x1 - 1, Area.y0 });
            Line(Color, { Area.x0 + 1, Area.y1 }, { Area.x1 - 1, Area.y1 });
            Line(Color, { Area.x0, Area.y0 }, { Area.x0, Area.y1 });
            Line(Color, { Area.x1, Area.y0 }, { Area.x1, Area.y1 });
        }
        template <> void Quad<true>(const texture_t Color, const vec4_t Area)
        {
            vec2_t Vertices[]{ {Area.x0, Area.y0}, {Area.x1, Area.y0}, {Area.x1, Area.y1}, {Area.x0, Area.y1} };
            if (Color.Alpha == 1.0f) Internal::fillPoly(Vertices, 4, [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)]);
            });
            else Internal::fillPoly(Vertices, 4, [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)], Color.Alpha);
            });
            Quad<false>(Color, Area);
        }
        void Line(const texture_t Color, const vec2_t Start, const vec2_t Stop)
        {
            vec2_t Vertices[]{ Start, Stop };
            if(Color.Alpha == 1.0f) Internal::fillPoly(Vertices, 2, [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)]);
            });
            else Internal::fillPoly(Vertices, 2, [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)], Color.Alpha);
            });
        }
        void Polygon(const texture_t Color, const std::vector<vec2_t> Vertices)
        {
            if(Color.Alpha == 1.0f) Internal::fillPoly(Vertices.data(), Vertices.size(), [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)]);
            });
            else Internal::fillPoly(Vertices.data(), Vertices.size(), [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)], Color.Alpha);
            });
        }
        void Circle(const texture_t Color, const vec2_t Position, float Radius)
        {
            const auto Clipped{ Internal::clipArea({ Position.x - Radius, Position.y - Radius, Position.x + Radius, Position.y + Radius}) };
            const int64_t Rad2{ int64_t(Radius * Radius) };
            const int64_t Diameter{ int64_t(Radius * 2) };

            for (int64_t i = 0; i < Rad2 << 2; ++i)
            {
                const int64_t tx{ int64_t((i % Diameter) - Radius) };
                const int64_t ty{ int64_t((i / Diameter) - Radius) };

                if (tx * tx + ty * ty <= Rad2)
                {
                    const int64_t X{ int64_t(Position.x) + tx };
                    const int64_t Y{ int64_t(Position.y) + ty };

                    if (X >= Clipped.x0 && X <= Clipped.x1 && Y >= Clipped.y0 && Y <= Clipped.y1)
                    {
                        if(Color.Alpha == 1.0f) Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)]);
                        else Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)], Color.Alpha);
                    }
                }
            }
        }
    }
    namespace Effectdraw
    {
        void Blur(const vec4_t Area)
        {
            const auto Clipped{ Internal::clipArea(Area) };
            const auto Scanlines{ Clipped.y1 - Clipped.y0 };
            const auto Linelength{ Clipped.x1 - Clipped.x0 };
            auto Buffer{ std::make_unique<Pixel_t[]>(Scanlines* Linelength) };

            // Copy the part of the buffer we want.
            for (size_t Y = 0; Y < Scanlines; ++Y)
            {
                std::memcpy(&Buffer[size_t(Y * Linelength)], &Canvas[size_t(Y * size_t(Resolution.x) + Clipped.x0)], Linelength * sizeof(Pixel_t));
            }

            for (size_t Y = 1; Y < Scanlines - 1; ++Y)
            {
                for (size_t X = 1; X < Linelength - 1; X++)
                {
                    Pixel_t Input[9]
                    {
                        Buffer[size_t((Y - 1) * Linelength + X - 1)],
                        Buffer[size_t((Y - 1) * Linelength + X + 1)],
                        Buffer[size_t((Y + 1) * Linelength + X - 1)],
                        Buffer[size_t((Y + 1) * Linelength + X + 1)],

                        Buffer[size_t((Y) * Linelength + X - 1)],
                        Buffer[size_t((Y) * Linelength + X + 1)],
                        Buffer[size_t((Y + 1) * Linelength + X)],
                        Buffer[size_t((Y + 1) * Linelength + X)],

                        Buffer[size_t((Y) * Linelength + X)]
                    };
                    Internal::setPixel(X + Clipped.x0, Y + Clipped.y0,
                    {
                        uint8_t((Buffer[8].BGR.B * 2 + Input[0].BGR.B + Input[1].BGR.B + Input[2].BGR.B + Input[3].BGR.B + Input[4].BGR.B + Input[5].BGR.B + Input[6].BGR.B + Input[7].BGR.B) / 9),
                        uint8_t((Buffer[8].BGR.G * 2 + Input[0].BGR.G + Input[1].BGR.G + Input[2].BGR.G + Input[3].BGR.G + Input[4].BGR.G + Input[5].BGR.G + Input[6].BGR.G + Input[7].BGR.G) / 9),
                        uint8_t((Buffer[8].BGR.R * 2 + Input[0].BGR.R + Input[1].BGR.R + Input[2].BGR.R + Input[3].BGR.R + Input[4].BGR.R + Input[5].BGR.R + Input[6].BGR.R + Input[7].BGR.R) / 9)
                    });
                }
            }
        }
    }

    // Basic textures.
    namespace Texture
    {
        std::atomic<uint32_t> Texturecount{ 0 };
        texture_t Creategradient(const size_t Steps, const rgba_t Color1, const rgba_t Color2)
        {
            texture_t Texture{ Texturecount++, Steps, 1, 1.0f, new Pixel_t[Steps] };
            size_t Index{};

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

                ((Pixel_t*)Texture.Data)[Index++] = Internal::fromRGBA(Blended);
            }
            for (float i = 0; i < 1; i += (1.0 / (Steps / 2)))
            {
                rgba_t Blended;
                Blended.R = (ColorB.R / 255 * i) + (ColorA.R / 255 * (1 - i));
                Blended.G = (ColorB.G / 255 * i) + (ColorA.G / 255 * (1 - i));
                Blended.B = (ColorB.B / 255 * i) + (ColorA.B / 255 * (1 - i));
                Blended.A = 1;

                ((Pixel_t*)Texture.Data)[Index++] = Internal::fromRGBA(Blended);
            }

            return Texture;
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
        void Quad(rgba_t Color, vec4_t Box, vec4_t Margin) {}
        void Line(rgba_t Color, vec4_t Box, vec4_t Margin) {}
        void Border(rgba_t Color, vec4_t Box, vec4_t Margin) {}
    }
}

#endif
