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
            for (size_t Y = std::max(0.0f, Clippingarea.y0); Y < std::min(Resolution.y, Clippingarea.y1); ++Y)
            {
                int32_t i{}, j{ int32_t(Count) - 1 }, Nodecount{};

                // Create a few nodes.
                for (i = 0; i < Count; ++i)
                {
                    if ((Vertices[i].y <= Y && Vertices[j].y >= Y) || (Vertices[j].y <= Y && Vertices[i].y >= Y))
                    {
                        if(Vertices[j].y - Vertices[i].y == 0) Nodes[Nodecount++] = Vertices[i].x;
                        else Nodes[Nodecount++] = Vertices[i].x + ((double)(Y - Vertices[i].y) / (Vertices[j].y - Vertices[i].y)) * (Vertices[j].x - Vertices[i].x);
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
                    if (Nodes[i] >= std::min(Resolution.x, Clippingarea.x1)) break;
                    if (Nodes[i + 1] >= std::min(0.0f, Clippingarea.x0))
                    {
                        if (Nodes[i] < std::max(Clippingarea.x0, 0.0f)) Nodes[i] = std::max(Clippingarea.x0, 0.0f);
                        if (Nodes[i + 1] > std::min(Clippingarea.x1, Resolution.x)) Nodes[i + 1] = std::min(Clippingarea.x1, Resolution.x);

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
        template<> void Triangle<false>(const rgba_t Color, const vec2_t a, const vec2_t b, const vec2_t c)
        {
            if (Color.A == 0.0f) return;
            Line(Color, { a.x, a.y }, { b.x, b.y });
            Line(Color, { b.x, b.y }, { c.x, c.y });
            Line(Color, { c.x, c.y }, { a.x, a.y });
        }
        template<> void Triangle<true>(const rgba_t Color, const vec2_t a, const vec2_t b, const vec2_t c)
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
            Line(Color, { Area.x0, Area.y0 }, { Area.x1, Area.y0 });
            Line(Color, { Area.x0, Area.y1 }, { Area.x1, Area.y1 });
            Line(Color, { Area.x0, Area.y0 + 1 }, { Area.x0, Area.y1 - 1 });
            Line(Color, { Area.x1, Area.y0 + 1 }, { Area.x1, Area.y1 - 1 });
        }
        template <> void Quad<true>(const rgba_t Color, const vec4_t Area)
        {
            if (Color.A == 0.0f) return;
            auto Pixel{ Internal::fromRGBA(Color) };
            vec2_t Vertices[]{ {Area.x0, Area.y0}, {Area.x1, Area.y0}, {Area.x1, Area.y1}, {Area.x0, Area.y1} };

            if(Color.A == 1.0f) Internal::fillPoly(Vertices, 4, [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel); });
            else Internal::fillPoly(Vertices, 4, [&](const size_t X, const size_t Y) { Internal::setPixel(X, Y, Pixel, Color.A); });
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
            Internal::fillPoly(Vertices, 3, [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)]);
            });
        }
        template <> void Quad<false>(const texture_t Color, const vec4_t Area)
        {
            Line(Color, { Area.x0, Area.y0 }, { Area.x1, Area.y0 });
            Line(Color, { Area.x0, Area.y1 }, { Area.x1, Area.y1 });
            Line(Color, { Area.x0, Area.y0 + 1 }, { Area.x0, Area.y1 - 1 });
            Line(Color, { Area.x1, Area.y0 + 1 }, { Area.x1, Area.y1 - 1 });
        }
        template <> void Quad<true>(const texture_t Color, const vec4_t Area)
        {
            vec2_t Vertices[]{ {Area.x0, Area.y0}, {Area.x1, Area.y0}, {Area.x1, Area.y1}, {Area.x0, Area.y1} };
            Internal::fillPoly(Vertices, 4, [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)]);
            });
        }
        void Line(const texture_t Color, const vec2_t Start, const vec2_t Stop)
        {
            vec2_t Vertices[]{ Start, Stop };
            Internal::fillPoly(Vertices, 2, [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)]);
            });
        }
        void Polygon(const texture_t Color, const std::vector<vec2_t> Vertices)
        {
            Internal::fillPoly(Vertices.data(), Vertices.size(), [&](const size_t X, const size_t Y)
            {
                Internal::setPixel(X, Y, ((Pixel_t *)Color.Data)[(Y % Color.Height) * Color.Width + (X % Color.Width)]);
            });
        }
    }

    // Basic textures.
    namespace Texture
    {
        std::atomic<uint32_t> Texturecount{ 0 };
        texture_t Creategradient(const size_t Steps, const rgba_t Color1, const rgba_t Color2)
        {
            texture_t Texture{ Texturecount++, Steps, 1, new Pixel_t[Steps] };
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






//namespace Rendering
//{
//    static std::queue<vec4_t> Invalidareas{};
//
//    static pixel24_t *Canvas{};
//    static HBITMAP Surface{};
//
//    // System-code interaction, assumes single-threaded sync.
//    void onPresent(const void *Handle)
//    {
//        StretchBlt((HDC)Handle, 0, 0, gWidth, gHeight, Surfacecontext, 0, 0, Resolution.x, Resolution.y, SRCCOPY);
//        //BitBlt((HDC)Handle, 0, 0, gWidth, gHeight, Surfacecontext, 0, 0, SRCCOPY);
//    }
//    void onRender()
//    {
//        if (!Canvas)
//        {
//            auto Devicecontext{ GetDC(NULL) };
//            BITMAPINFO Format{};
//
//            // Bitmap format.
//            Format.bmiHeader.biSize = sizeof(BITMAPINFO);
//            Format.bmiHeader.biHeight =  -(Resolution.y + 1);
//            Format.bmiHeader.biWidth = Resolution.x;
//            Format.bmiHeader.biCompression = BI_RGB;
//            Format.bmiHeader.biBitCount = 24;
//            Format.bmiHeader.biPlanes = 1;
//
//            // Cleanup any allocated memory.
//            if (Surface) DeleteObject(Surface);
//            if (Surfacecontext) DeleteDC(Surfacecontext);
//
//            // Create the new surface.
//            Surface = CreateDIBSection(Devicecontext, &Format, DIB_RGB_COLORS, (void **)&Canvas, NULL, 0);
//            Surfacecontext = CreateCompatibleDC(Devicecontext);
//            SelectObject(Surfacecontext, Surface);
//
//            // C-style cleanup needed.
//            DeleteDC(Devicecontext);
//        }
//
//        // Update the elements if the resolution changed.
//        if (lWidth != gWidth || lHeight != gHeight)
//        {
//            getRootelement()->Renderbox = { 0, 0, Resolution.x, Resolution.y };
//            getRootelement()->Boundingbox = { 0, 0, gWidth, gHeight };
//            getRootelement()->onModifiedstate(getRootelement());
//            lWidth = gWidth; lHeight = gHeight;
//        }
//
//        // Render the element-tree.
//        std::function<void(Element_t *, vec4_t Clip)> Lambda = [&](Element_t *Parent, vec4_t Clip) -> void
//        {
//            Parent->onRender(Parent, Clip);
//            for (const auto &Child : Parent->Children) Lambda(Child, Clip);
//        };
//
//        // Update all areas.
//        while (!Invalidareas.empty())
//        {
//            Lambda(getRootelement(), Invalidareas.front());
//            Invalidareas.pop();
//        }
//    }
//
//    // User-code interaction.
//    namespace Draw
//    {
//        // Windows wants to use BGR.
//        ainline pixel24_t fromRGBA(const rgba_t Color)
//        {
//            return
//            {
//                uint8_t(Color.B <= 1 ? Color.B * 255 : Color.B),
//                uint8_t(Color.G <= 1 ? Color.G * 255 : Color.G),
//                uint8_t(Color.R <= 1 ? Color.R * 255 : Color.R)
//            };
//        }
//        ainline pixel24_t Blend(const pixel24_t Base, const pixel24_t Overlay, const double Alpha)
//        {
//            return
//            {
//                uint8_t(Base.BGR.B * (1.0 - Alpha) + Overlay.BGR.B * Alpha),
//                uint8_t(Base.BGR.G * (1.0 - Alpha) + Overlay.BGR.G * Alpha),
//                uint8_t(Base.BGR.R * (1.0 - Alpha) + Overlay.BGR.R * Alpha)
//            };
//        }
//        ainline void Setpixel(const pixel24_t Input, const size_t X, const size_t Y, const double Alpha)
//        {
//            Canvas[Y * size_t(Resolution.x) + X] = Blend(Canvas[Y * size_t(Resolution.x) + X], Input, Alpha);
//        }
//        ainline void Blendedfill(const pixel24_t Input, const vec4_t Box, const double Alpha)
//        {
//            for (int64_t Y = std::clamp(Box.y0, -1.0f, Resolution.y); Y <= std::clamp(Box.y1, -1.0f, Resolution.y); ++Y)
//            {
//                for (int64_t X = std::clamp(Box.x0, -1.0f, Resolution.x); X <= std::clamp(Box.x1, -1.0f, Resolution.x); ++X)
//                {
//                    if (X >= 0 && Y >= 0) Setpixel(Input, X, Y, Alpha);
//                }
//            }
//        }
//        ainline void Setpixel(const pixel24_t Input, const size_t X, const size_t Y)
//        {
//            Canvas[Y * size_t(Resolution.x) + X] = Input;
//        }
//        ainline void Solidfill(const pixel24_t Input, const vec4_t Box)
//        {
//            for (int64_t Y = std::clamp(Box.y0, -1.0f, Resolution.y); Y <= std::clamp(Box.y1, -1.0f, Resolution.y); ++Y)
//            {
//                for (int64_t X = std::clamp(Box.x0, -1.0f, Resolution.x); X <= std::clamp(Box.x1, -1.0f, Resolution.x); ++X)
//                {
//                    if (X >= 0 && Y >= 0) Setpixel(Input, X, Y);
//                }
//            }
//        }
//
//        // Basic drawing.
//        void Quad(const rgba_t Color, const vec4_t Box, const vec4_t Clip)
//        {
//            // Invisible element.
//            if (Color.A == 0.0) return;
//
//            // Clipped area.
//            const vec4_t Area
//            {
//                std::max(Box.x0, Clip.x0), std::max(Box.y0, Clip.y0),
//                std::min(Box.x1, Clip.x1), std::min(Box.y1, Clip.y1)
//            };
//
//            // Solid element.
//            if (Color.A == 1.0) Solidfill(fromRGBA(Color), Area);
//
//            // Transparent element.
//            else return Blendedfill(fromRGBA(Color), Area, Color.A);
//        }
//        void Line(const rgba_t Color, const vec4_t Box, const vec4_t Clip)
//        {
//            // Invisible element.
//            if (Color.A == 0.0) return;
//
//            // Clipped area.
//            vec4_t Area{ Box };
//
//            // Check if the line is steep and invert.
//            const bool Steep{ std::abs(Area.x0 - Area.x1) < std::abs(Area.y0 - Area.y1) };
//            if (Steep)
//            {
//                std::swap(Area.x0, Area.y0);
//                std::swap(Area.x1, Area.y1);
//            }
//            if (Area.x0 > Area.x1) std::swap(Area.x0, Area.x1);
//            if (Area.y0 > Area.y1) std::swap(Area.y0, Area.y1);
//
//            const auto DeltaX{ Area.x1 - Area.x0 };
//            const auto DeltaY{ Area.y1 - Area.y0 };
//
//            // Straight line, just quad-fill.
//            if (DeltaX == 0.0 || DeltaY == 0.0) return Quad(Color, Area);
//
//            // Draw the actual line.
//            const auto Pixel = fromRGBA(Color);
//            int64_t Y{ int64_t(Area.y0) }, Error{};
//            const int64_t Deltaerror{ int64_t(std::abs(DeltaY) * 2) };
//            for (int64_t X = Area.x0; X <= Area.x1; ++X)
//            {
//                // Only draw inside the canvas.
//                if (X >= 0 && Y >= 0)
//                {
//                    // Only draw inside the clipped area.
//                    if (X >= Clip.x0 && X <= Clip.x1 && Y >= Clip.y0 && Y <= Clip.y1)
//                    {
//                        // Invert the coordinates if too steep.
//                        if (Steep) Setpixel(Pixel, Y, X, Color.A);
//                        else Setpixel(Pixel, X, Y, Color.A);
//                    }
//                }
//
//                // Update the error offset.
//                Error += Deltaerror;
//                if (Error > DeltaX)
//                {
//                    Y += Area.y1 > Area.y0 ? 1 : -1;
//                    Error -= DeltaX * 2;
//                }
//            }
//        }
//        void Border(const rgba_t Color, const vec4_t Box, const vec4_t Clip)
//        {
//            Line(Color, { Box.x0, Box.y0, Box.x1, Box.y0 }, Clip);
//            Line(Color, { Box.x0, Box.y1, Box.x1, Box.y1 }, Clip);
//
//            Line(Color, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 }, Clip);
//            Line(Color, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 }, Clip);
//        }
//
//        // Textured drawing.
//        void Texturedquad(const texture_t Texture, const vec4_t Box, const vec4_t Clip)
//        {
//            const auto Colors{ Texture::getGradient(Texture) }; if (!Colors) return;
//            const size_t Colorcount{ Colors->size() };
//            size_t Colorindex{};
//
//            // Fill the quad.
//            for (int64_t Y = std::clamp(Box.y0, -1.0f, Resolution.y); Y <= std::clamp(Box.y1, -1.0f, Resolution.y); ++Y)
//            {
//                for (int64_t X = std::clamp(Box.x0, -1.0f, Resolution.x); X <= std::clamp(Box.x1, -1.0f, Resolution.x); ++X)
//                {
//                    // Only draw inside the canvas.
//                    if (X >= 0 && Y >= 0)
//                    {
//                        // Only draw inside the clipped area.
//                        if (X >= Clip.x0 && X <= Clip.x1 && Y >= Clip.y0 && Y <= Clip.y1)
//                        {
//                            Setpixel((*Colors)[Colorindex++ % Colorcount], X, Y);
//                        }
//                    }
//                }
//            }
//        }
//        void Texturedline(const texture_t Texture, const vec4_t Box, const vec4_t Clip)
//        {
//            // Clipped area.
//            vec4_t Area{ Box };
//
//            // Check if the line is steep and invert.
//            const bool Steep{ std::abs(Area.x0 - Area.x1) < std::abs(Area.y0 - Area.y1) };
//            if (Steep)
//            {
//                std::swap(Area.x0, Area.y0);
//                std::swap(Area.x1, Area.y1);
//            }
//            if (Area.x0 > Area.x1) std::swap(Area.x0, Area.x1);
//            if (Area.y0 > Area.y1) std::swap(Area.y0, Area.y1);
//
//            const auto DeltaX{ Area.x1 - Area.x0 };
//            const auto DeltaY{ Area.y1 - Area.y0 };
//
//            // Straight line, just quad-fill.
//            if (DeltaX == 0.0 || DeltaY == 0.0) return Texturedquad(Texture, Box, Clip);
//
//            const auto Colors{ Texture::getGradient(Texture) }; if (!Colors) return;
//            const size_t Colorcount{ Colors->size() };
//            size_t Colorindex{};
//
//            // Draw the actual line.
//            int64_t Y{ int64_t(Area.y0) }, Error{};
//            const int64_t Deltaerror{ int64_t(std::abs(DeltaY) * 2) };
//            for (int64_t X = Area.x0; X <= Area.x1; ++X)
//            {
//                // Only draw inside the canvas.
//                if (X >= 0 && Y >= 0)
//                {
//                    // Only draw inside the clipped area.
//                    if (X >= Clip.x0 && X <= Clip.x1 && Y >= Clip.y0 && Y <= Clip.y1)
//                    {
//                        // Invert the coordinates if too steep.
//                        if(Steep) Setpixel((*Colors)[Colorindex++ % Colorcount], Y, X);
//                        else Setpixel((*Colors)[Colorindex++ % Colorcount], X, Y);
//                    }
//                }
//
//                // Update the error offset.
//                Error += Deltaerror;
//                if (Error > DeltaX)
//                {
//                    Y += Area.y1 > Area.y0 ? 1 : -1;
//                    Error -= DeltaX * 2;
//                }
//            }
//        }
//        void Texturedborder(const texture_t Texture, const vec4_t Box, const vec4_t Clip)
//        {
//            Texturedline(Texture, { Box.x0, Box.y0, Box.x1, Box.y0 }, Clip);
//            Texturedline(Texture, { Box.x0, Box.y1, Box.x1, Box.y1 }, Clip);
//
//            Texturedline(Texture, { Box.x0, Box.y0 + 1, Box.x0, Box.y1 - 1 }, Clip);
//            Texturedline(Texture, { Box.x1, Box.y0 + 1, Box.x1, Box.y1 - 1 }, Clip);
//        }
//    }
//    namespace Texture
//    {
//        std::atomic<uint32_t> Texturecount{ 0 };
//        std::unordered_map<texture_t, std::vector<pixel24_t>> Texturemap;
//        texture_t Creategradient(const size_t Steps, const rgba_t Color1, const rgba_t Color2)
//        {
//            auto TextureID{ Texturecount++ };
//            auto Entry = &Texturemap[TextureID];
//            Entry->reserve(Steps);
//
//            // Normalize the colors.
//            const rgba_t ColorA{ Color1.R <= 1 ? Color1.R * 255 : Color1.R, Color1.G <= 1 ? Color1.G * 255 : Color1.G, Color1.B <= 1 ? Color1.B * 255 : Color1.B };
//            const rgba_t ColorB{ Color2.R <= 1 ? Color2.R * 255 : Color2.R, Color2.G <= 1 ? Color2.G * 255 : Color2.G, Color2.B <= 1 ? Color2.B * 255 : Color2.B };
//
//            // Generate half the steps from each direction.
//            for (float i = 0; i < 1; i += (1.0 / (Steps / 2)))
//            {
//                rgba_t Blended;
//                Blended.R = (ColorA.R / 255 * i) + (ColorB.R / 255 * (1 - i));
//                Blended.G = (ColorA.G / 255 * i) + (ColorB.G / 255 * (1 - i));
//                Blended.B = (ColorA.B / 255 * i) + (ColorB.B / 255 * (1 - i));
//                Blended.A = 1;
//
//                Entry->push_back(Draw::fromRGBA(Blended));
//            }
//            for (float i = 0; i < 1; i += (1.0 / (Steps / 2)))
//            {
//                rgba_t Blended;
//                Blended.R = (ColorB.R / 255 * i) + (ColorA.R / 255 * (1 - i));
//                Blended.G = (ColorB.G / 255 * i) + (ColorA.G / 255 * (1 - i));
//                Blended.B = (ColorB.B / 255 * i) + (ColorA.B / 255 * (1 - i));
//                Blended.A = 1;
//
//                Entry->push_back(Draw::fromRGBA(Blended));
//            }
//
//            return TextureID;
//        }
//        std::vector<pixel24_t> *getGradient(const texture_t ID)
//        {
//            if (const auto Item = Texturemap.find(ID); Item != Texturemap.end())
//                return &Item->second;
//            return nullptr;
//        }
//    }
//
//    void Invalidatearea(const vec4_t Box)
//    {
//        Invalidareas.push(Box);
//    }
//}

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
