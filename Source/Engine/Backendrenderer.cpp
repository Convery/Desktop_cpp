/*
    Initial author: Convery (tcn@ayria.se)
    Started: 12-07-2018
    License: MIT

    Turns code into colors on the screen =P
*/

#include "../Stdinclude.hpp"
#define STBI_ONLY_PNG
//#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "../Utility/stb_image.h"
#include <emmintrin.h>

#if !defined(_WIN32)
    #error Non-windows abstraction is not implemented (yet!)
#else
namespace Engine::Rendering
{
    namespace Canvas
    {
        constexpr size_t Segments{ 5 };
        pixel24_t *Framebuffer{};
        BITMAPINFO Format{};
        size_t Buffersize{};
    }

    const point4_t Defaultclippingarea{ -1, -1, -1, -1 };
    point4_t Globalclippingarea{ Defaultclippingarea };
    point2_t gRenderingresolution{100, 100};
    point4_t Currentclippingarea{};

    // Invalidate the area that needs to be redrawn.
    void Invalidatearea(point4_t Area)
    {
        Globalclippingarea.x0 = std::max(Globalclippingarea.x0, Area.x0);
        Globalclippingarea.y0 = std::max(Globalclippingarea.y0, Area.y0);
        Globalclippingarea.x1 = std::max(Globalclippingarea.x1, Area.x1);
        Globalclippingarea.y1 = std::max(Globalclippingarea.y1, Area.y1);
    }

    // Process elements, render, and present to the context.
    void onRender(const void *Context)
    {
        // Track the frame-time, should be less than 33ms.
        static auto Lastframe{ std::chrono::high_resolution_clock::now() };
        const auto Currentframe{ std::chrono::high_resolution_clock::now() };
        const auto Deltatime{ std::chrono::duration<double>(Currentframe - Lastframe).count() };
        Lastframe = Currentframe;

        // Helper to save my fingers.
        std::function<void(Element_t *)> Tick = [&](Element_t *This) -> void
        {
            if (This->onFrame) This->onFrame(This, Deltatime);
            for (const auto &Item : This->Childelements) Tick(Item);
        };

        // Update all elements.
        assert(gRootelement);
        Tick(gRootelement);

        // If the global area is unmodified, don't render anything.
        if (0 == std::memcmp(Globalclippingarea.Raw, Defaultclippingarea.Raw, sizeof(point4_t))) return;

        // Render a part of the window.
        auto Rendersegment = [&](point4_t Clippingarea) -> void
        {
            Currentclippingarea = Clippingarea;

            // Clear the buffer.
            std::memset(Canvas::Framebuffer, 0xFF, Canvas::Buffersize);

            // Helper to save my fingers.
            std::function<void(Element_t *)> Render = [&](Element_t *This) -> void
            {
                if (This->onRender) This->onRender(This);
                for (const auto &Item : This->Childelements) Render(Item);
            };

            // Render all elements.
            assert(gRootelement);
            Render(gRootelement);

            // Debugging borders for the quadrants.
            if constexpr(Build::Debug::isDebugging)
            {
                //Draw::Quad<true>({ 0xFF, 0x00, 0xFF, 1 }, Clippingarea);
            }
        };

        // Don't render off-screen.
        Globalclippingarea.x1 = std::clamp(Globalclippingarea.x1, {}, int16_t(gRenderingresolution.x));
        Globalclippingarea.y1 = std::clamp(Globalclippingarea.y1, {}, int16_t(gRenderingresolution.y));

        // Render each dirty segment.
        for (int16_t i = 0; i < Canvas::Segments; ++i)
        {
            // Out of bounds.
            if (Globalclippingarea.x0 > int16_t(gRenderingresolution.x)) continue;
            if (Globalclippingarea.y1 < int16_t(gRenderingresolution.y / Canvas::Segments * i)) continue;
            if (Globalclippingarea.y0 > int16_t(gRenderingresolution.y / Canvas::Segments * (i + 1))) continue;

            Rendersegment({ 0, int16_t(gRenderingresolution.y / Canvas::Segments * i), int16_t(gRenderingresolution.x - 1), int16_t(gRenderingresolution.y / Canvas::Segments * (i + 1) - 1) });
            StretchDIBits(HDC(Context), 0, int(gRenderingresolution.y / Canvas::Segments) * i, gRenderingresolution.x, int(gRenderingresolution.y / Canvas::Segments), 0, 0,
                gRenderingresolution.x, gRenderingresolution.y / Canvas::Segments, Canvas::Framebuffer, &Canvas::Format, DIB_RGB_COLORS, SRCCOPY);
        }

        // Reset the dirty area.
        //Globalclippingarea = Defaultclippingarea;
    }

    // Create the framebuffer if needed.
    void Createframebuffer(const point2_t Size)
    {
        auto Devicecontext{ GetDC(NULL) };
        gRenderingresolution = Size;

        // Bitmap format, upside-down because Windows.
        Canvas::Format.bmiHeader.biHeight = -(gRenderingresolution.y / Canvas::Segments);
        Canvas::Format.bmiHeader.biWidth = gRenderingresolution.x;
        Canvas::Format.bmiHeader.biSize = sizeof(BITMAPINFO);
        Canvas::Format.bmiHeader.biCompression = BI_RGB;
        Canvas::Format.bmiHeader.biBitCount = 24;
        Canvas::Format.bmiHeader.biPlanes = 1;

        // Create the new buffer padded to a multiple of 128bit.
        if (Canvas::Framebuffer) std::thread([=]() { std::this_thread::sleep_for(std::chrono::seconds(1)); HeapFree(GetProcessHeap(), NULL, Canvas::Framebuffer); }).detach();
        Canvas::Buffersize = gRenderingresolution.y / Canvas::Segments * gRenderingresolution.x * sizeof(pixel24_t);
        Canvas::Buffersize += 16 + (Canvas::Buffersize % 16) ? (16 - (Canvas::Buffersize % 16)) : 0;
        Canvas::Framebuffer = (pixel24_t *)HeapAlloc(GetProcessHeap(), NULL, Canvas::Buffersize);

        // C-style cleanup needed.
        DeleteDC(Devicecontext);
    }
}
#endif

// Drawing primitives.
namespace Engine::Rendering::Draw::Internal
{
    // Core components.
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
    ainline void setPixel(point2_t Position, const pixel32_t Color)
    {
        // Buffer-width is currently equal to the rendering-resolution.
        Position.y %= gRenderingresolution.y / Canvas::Segments;
        // Position.x %= gRenderingresolution.x;

        // Alpha is always the last entry.
        if (Color.Raw[3] == 0xFF)
        {
            std::memcpy(Canvas::Framebuffer[Position.y * gRenderingresolution.x + Position.x].Raw, Color.Raw, sizeof(pixel24_t));
        }
        else
        {
            #define BLEND(A, B) A += int32_t((((B - A) * Color.Raw[3]))) >> 8;
            BLEND(Canvas::Framebuffer[Position.y * gRenderingresolution.x + Position.x].Raw[0], Color.Raw[0]);
            BLEND(Canvas::Framebuffer[Position.y * gRenderingresolution.x + Position.x].Raw[1], Color.Raw[1]);
            BLEND(Canvas::Framebuffer[Position.y * gRenderingresolution.x + Position.x].Raw[2], Color.Raw[2]);
        }
    }
    ainline void setPixel(const point2_t Position, const pixel24_t Color)
    {
        return setPixel(Position, { Color.Raw[0], Color.Raw[1], Color.Raw[2], 0xFF });
    }

    // Draw a simple line, should add some anti-aliasing later.
    template<typename CB = std::function<void(const point2_t Position)>>
    ainline void drawLine(point2_t Start, point2_t Stop, CB Callback)
    {
        int16_t X, Y, Error{};

        // Always draw left to right, top to bottom.
        if (Start.x > Stop.x) std::swap(Start.x, Stop.x);
        if (Start.y > Stop.y) std::swap(Start.y, Stop.y);

        // Straight line case.
        if (Start.x == Stop.x)
        {
            for (Y = Start.y; Y != Stop.y; Y += Start.y <= Stop.y ? 1 : -1)
            {
                Callback({ Start.x, Y });
            }
            Callback(Stop);
            return;
        }
        if (Start.y == Stop.y)
        {
            for (X = Start.x; X != Stop.x; X += Start.x <= Stop.x ? 1 : -1)
            {
                Callback({ X, Start.y });
            }
            Callback(Stop);
            return;
        }

        // How much to move per iteration.
        const auto DeltaX{ int16_t(Stop.x - Start.x) };
        const auto DeltaY{ int16_t(Stop.y - Start.y) };

        // If the line is steep.
        if (DeltaX >= DeltaY)
        {
            for (X = Start.x, Y = Start.y; X <= Stop.x; ++X)
            {
                Callback({ X, Y });
                Error += DeltaY;
                if (Error >= DeltaX)
                {
                    Error -= DeltaX; Y++;
                    Callback({ X, Y });
                }
            }
            Callback(Stop);
        }
        else
        {
            for (X = Start.x, Y = Start.y; Y <= Stop.y; ++Y)
            {
                Callback({ X, Y });
                Error += DeltaX;
                if (Error >= DeltaY)
                {
                    Error -= DeltaY; Y++;
                    Callback({ X, Y });
                }
            }
            Callback(Stop);
        }
    }

    // The simplest drawing-call ever.
    template<typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void fillRect(const point4_t Area, CB Callback)
    {
        const int16_t Width{ std::min(Area.x1, Currentclippingarea.x1) - std::max(Area.x0, Currentclippingarea.x0) };
        for (int16_t Y = std::max(Area.y0, Currentclippingarea.y0); Y < std::min(Area.y1, Currentclippingarea.y1); ++Y)
            Callback({ std::max(Area.x0, Currentclippingarea.x0), Y }, Width);
    }

    // Outline and fill polygons.
    template<typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void fillPolygon(const vec2_t *Vertices, const size_t Verticecount, CB Callback)
    {
        auto Nodes{ std::make_unique<int16_t[]>(Verticecount + 1) };

        // For each row in the canvas.
        for (int16_t Y = Currentclippingarea.y0; Y < Currentclippingarea.y1; ++Y)
        {
            int16_t i{}, j{ int16_t(Verticecount - 1) }, Nodecount{};

            // Create a few nodes.
            for (i = 0; i < int16_t(Verticecount); ++i)
            {
                if ((Vertices[i].y <= Y && Vertices[j].y >= Y) || (Vertices[j].y <= Y && Vertices[i].y >= Y))
                {
                    if (Vertices[j].y - Vertices[i].y == 0) Nodes[Nodecount++] = int16_t(Vertices[i].x + (Y - Vertices[i].y) * (Vertices[j].x - Vertices[i].x));
                    else Nodes[Nodecount++] = int16_t(Vertices[i].x + ((Y - Vertices[i].y) / (Vertices[j].y - Vertices[i].y)) * (Vertices[j].x - Vertices[i].x));
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
                if (Nodes[i] > std::min(gRenderingresolution.x, Currentclippingarea.x1)) break;
                if (Nodes[i + 1] >= std::min({}, Currentclippingarea.x0))
                {
                    if (Nodes[i] < std::max(Currentclippingarea.x0, {})) Nodes[i] = std::max(Currentclippingarea.x0, {});
                    if (Nodes[i + 1] > std::min(Currentclippingarea.x1, gRenderingresolution.x)) Nodes[i + 1] = std::min(Currentclippingarea.x1, gRenderingresolution.x);

                    Callback({ Nodes[i], Y }, Nodes[i + 1] - Nodes[i] + 1);
                }
            }
        }
    }
    template<typename CB = std::function<void(const point2_t Position)>>
    ainline void outlinePolygon(const vec2_t *Vertices, const size_t Verticecount, CB Callback)
    {
        for (size_t i = 0; i < Verticecount - 1; i++)
        {
            drawLine({ int16_t(Vertices[i].x), int16_t(Vertices[i].y) }, { int16_t(Vertices[i + 1].x), int16_t(Vertices[i + 1].y) }, Callback);
        }
    }

    // Outline and fill circles.
    template <bool Fill = true, typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void drawCircle(const point2_t Position, const float Radius, CB Callback)
    {
        const double R2{ Radius * Radius };
        int16_t X{}, Y{ int16_t(Radius) };

        // Pre-calculate the clipped area.
        const point4_t Area
        {
            int16_t(std::max((float)Position.x - Radius, (float)Currentclippingarea.x0)),
            int16_t(std::max((float)Position.y - Radius, (float)Currentclippingarea.y0)),
            int16_t(std::min((float)Position.x + Radius, (float)Currentclippingarea.x1)),
            int16_t(std::min((float)Position.y + Radius, (float)Currentclippingarea.y1))
        };

        // Helpers to keep the algorithm cleaner.
        auto doCallback = [&Area, &Callback](const point2_t Position, const size_t Length = 1) -> void
        {
            // Outside of the clipping area.
            if (Position.y < Area.y0 || Position.y >= Area.y1) return;
            const point2_t Scanline{ std::max(Position.x, Area.x0), std::min(int16_t(Position.x + Length), Area.x1) };
            Callback({ Scanline.x, Position.y }, std::max(int16_t(Scanline.y - Scanline.x), int16_t(1)));
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
            doDrawing(Position, { X, Y });
            X++; Y = int16_t(std::sqrt(R2 - X * X) + 0.5f);
        }

        // Precision..
        if (X == Y)
        {
            doDrawing(Position, { X, Y });
        }
    }
    template<typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void fillCircle(const point2_t Position, const float Radius, CB Callback)
    {
        return drawCircle<true>(Position, Radius, Callback);
    }
    template<typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void outlineCircle(const point2_t Position, const float Radius, CB Callback)
    {
        return drawCircle<false>(Position, Radius, Callback);
    }
}

// Draw-calls for the elements that are called every frame.
namespace Engine::Rendering::Draw
{
    template <bool Outline> void Circle(const texture_t Color, const point2_t Position, const float Radius)
    {
        if (Outline)
        {
            Internal::outlineCircle(Position, Radius, [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                {
                    if (Color.Pixelsize == sizeof(pixel32_t)) Internal::setPixel({ Position.x + i, Position.y }, ((pixel32_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x + i % Color.Dimensions.x]);
                    else Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x + i % Color.Dimensions.x]);
                }
            });
        }
        else
        {
            Internal::fillCircle(Position, Radius, [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                {
                    if (Color.Pixelsize == sizeof(pixel32_t)) Internal::setPixel({ Position.x + i, Position.y }, ((pixel32_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x + i % Color.Dimensions.x]);
                    else Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x + i % Color.Dimensions.x]);
                }
            });
        }
    }
    template <bool Outline> void Circle(const rgba_t Color, const point2_t Position, const float Radius)
    {
        const auto Pixel{ Internal::fromRGBA(Color) };
        if (Pixel.Raw[3] == 0) return;

        if (Outline)
        {
            Internal::outlineCircle(Position, Radius, [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel);
            });
        }
        else
        {
            Internal::fillCircle(Position, Radius, [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel);
            });
        }
    }
    template <bool Outline> void Polygon(const texture_t Color, const std::vector<vec2_t> Vertices)
    {
        Internal::outlinePolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position) -> void
        {
            if (Color.Pixelsize == sizeof(pixel32_t)) Internal::setPixel({ Position.x, Position.y }, ((pixel32_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x % Color.Dimensions.x]);
            else Internal::setPixel({ Position.x, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x % Color.Dimensions.x]);
        });
        if (!Outline)
        {
            Internal::fillPolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                {
                    if (Color.Pixelsize == sizeof(pixel32_t)) Internal::setPixel({ Position.x + i, Position.y }, ((pixel32_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x + i % Color.Dimensions.x]);
                    else Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x + i % Color.Dimensions.x]);
                }
            });
        }
    }
    template <bool Outline> void Polygon(const rgba_t Color, const std::vector<vec2_t> Vertices)
    {
        const auto Pixel{ Internal::fromRGBA(Color) };
        if (Pixel.Raw[3] == 0) return;

        Internal::outlinePolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, Pixel);
        });
        if (!Outline)
        {
            Internal::fillPolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel);
            });
        }
    }
    template <bool Outline> void Quad(const texture_t Color, const point4_t Area)
    {
        // Check if the area is even valid.
        if (Currentclippingarea.x0 > Area.x1 || Currentclippingarea.x1 < Area.x0 || Currentclippingarea.y0 > Area.y1 || Currentclippingarea.y1 < Area.y0)
        {
            return;
        }

        #pragma warning(suppress: 4838)
        const vec2_t Vertices[] =
        {
            { std::clamp(Area.x0, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y0, Currentclippingarea.y0, Currentclippingarea.y1) },
            { std::clamp(Area.x1, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y0, Currentclippingarea.y0, Currentclippingarea.y1) },
            { std::clamp(Area.x1, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y1, Currentclippingarea.y0, Currentclippingarea.y1) },
            { std::clamp(Area.x0, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y1, Currentclippingarea.y0, Currentclippingarea.y1) },
            { std::clamp(Area.x0, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y0, Currentclippingarea.y0, Currentclippingarea.y1) }
        };

        Internal::outlinePolygon(Vertices, 5, [&](const point2_t Position) -> void
        {
            if (Color.Pixelsize == sizeof(pixel32_t)) Internal::setPixel({ Position.x, Position.y }, ((pixel32_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x % Color.Dimensions.x]);
                    else Internal::setPixel({ Position.x, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x % Color.Dimensions.x]);
        });
        if (!Outline)
        {
            Internal::fillRect(Area, [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                {
                    if (Color.Pixelsize == sizeof(pixel32_t)) Internal::setPixel({ Position.x + i, Position.y }, ((pixel32_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x + i % Color.Dimensions.x]);
                    else Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x + i % Color.Dimensions.x]);
                }
            });
        }
    }
    template <bool Outline> void Quad(const rgba_t Color, const point4_t Area)
    {
        // Check if the area is even valid.
        if (Currentclippingarea.x0 > Area.x1 || Currentclippingarea.x1 < Area.x0 || Currentclippingarea.y0 > Area.y1 || Currentclippingarea.y1 < Area.y0)
        {
            return;
        }

        #pragma warning(suppress: 4838)
        const vec2_t Vertices[] =
        {
            { std::clamp(Area.x0, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y0, Currentclippingarea.y0, Currentclippingarea.y1) },
            { std::clamp(Area.x1, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y0, Currentclippingarea.y0, Currentclippingarea.y1) },
            { std::clamp(Area.x1, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y1, Currentclippingarea.y0, Currentclippingarea.y1) },
            { std::clamp(Area.x0, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y1, Currentclippingarea.y0, Currentclippingarea.y1) },
            { std::clamp(Area.x0, Currentclippingarea.x0, Currentclippingarea.x1), std::clamp(Area.y0, Currentclippingarea.y0, Currentclippingarea.y1) }
        };
        const auto Pixel{ Internal::fromRGBA(Color) };
        if (Pixel.Raw[3] == 0) return;

        Internal::outlinePolygon(Vertices, 5, [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, Pixel);
        });
        if (!Outline)
        {
            Internal::fillRect(Area, [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel);
            });
        }
    }
    void Line(const texture_t Color, const point2_t Start, const point2_t Stop)
    {
        Internal::drawLine(Start, Stop, [&](const point2_t Position) -> void
        {
            if (Color.Pixelsize == sizeof(pixel32_t)) Internal::setPixel({ Position.x, Position.y }, ((pixel32_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x % Color.Dimensions.x]);
            else Internal::setPixel({ Position.x, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Dimensions.y) * Color.Dimensions.x + Position.x % Color.Dimensions.x]);
        });
    }
    void Line(const rgba_t Color, const point2_t Start, const point2_t Stop)
    {
        const auto Pixel{ Internal::fromRGBA(Color) };
        if (Pixel.Raw[3] == 0) return;

        Internal::drawLine(Start, Stop, [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, Pixel);
        });
    }

    // Special drawing.
    void PNG(const unsigned char *Data, const size_t Size, const point4_t Area)
    {
        int Width, Height, Channels;
        const auto Image = stbi_load_from_memory(Data, Size, &Width, &Height, &Channels, 0);

        if (Channels == 3)
        {
            const auto Pixels = (pixel24_t *)Image;

            for (int16_t Y = std::max(Area.y0, Currentclippingarea.y0); Y < std::min(int16_t(Y + Height), Currentclippingarea.y1); ++Y)
            {
                for (int16_t X = std::max(Area.x0, Currentclippingarea.x0); X < std::min(int16_t(X + Width), Currentclippingarea.x1); ++X)
                {
                    const auto Offset{ (Y % Height) * Width + (X % Width) };

                    // Windows expects BGR.
                    std::swap(Pixels[Offset].Raw[0], Pixels[Offset].Raw[2]);
                    Internal::setPixel({ X, Y }, Pixels[Offset]);
                }
            }
        }
        else if (Channels == 4)
        {
            const auto Pixels = (pixel32_t *)Image;

            for (int16_t Y = std::max(Area.y0, Currentclippingarea.y0); Y < std::min(int16_t(Y + Height), Currentclippingarea.y1); ++Y)
            {
                for (int16_t X = std::max(Area.x0, Currentclippingarea.x0); X < std::min(int16_t(X + Width), Currentclippingarea.x1); ++X)
                {
                    const auto Offset{ (Y % Height) * Width + (X % Width) };

                    // Windows expects BGR.
                    std::swap(Pixels[Offset].Raw[0], Pixels[Offset].Raw[2]);
                    Internal::setPixel({ X, Y }, Pixels[Offset]);
                }
            }
        }
    }
    void PNGFile(const std::string &&Filename, const point4_t Area)
    {
        // Check if the area is even valid.
        if (Currentclippingarea.x0 > Area.x1 || Currentclippingarea.x1 < Area.x0 || Currentclippingarea.y0 > Area.y1 || Currentclippingarea.y1 < Area.y0)
        {
            return;
        }

        // Load the image into memory.
        int Imagewidth, Imageheight, Imagechannels;
        const auto Image = stbi_load(Filename.c_str(), &Imagewidth, &Imageheight, &Imagechannels, 0);

        // Set up the constraints.
        point2_t Height{ std::max(Area.y0, Currentclippingarea.y0), std::min(int16_t(std::max(Area.y0, Currentclippingarea.y0) + Imageheight), Currentclippingarea.y1) };
        point2_t Width{ std::max(Area.x0, Currentclippingarea.x0), std::min(int16_t(std::max(Area.x0, Currentclippingarea.x0) + Imagewidth), Currentclippingarea.x1) };

        // No alpha channel.
        if (Imagechannels == 3)
        {
            const auto Pixels = (pixel24_t *)Image;

            for (int16_t Y = Height.x; Y <= Height.y; ++Y)
            {
                for (int16_t X = Width.x; X < Width.y; ++X)
                {
                    // Windows expects BGR.
                    const auto Offset{ (Y - Area.y0) * Imagewidth + (X - Area.x0) };
                    Internal::setPixel({ X, Y }, { Pixels[Offset].RGB.B, Pixels[Offset].RGB.G, Pixels[Offset].RGB.R, 0xFF });
                }
            }
        }

        // With alpha channel.
        else if (Imagechannels == 4)
        {
            const auto Pixels = (pixel32_t *)Image;

            for (int16_t Y = Height.x; Y <= Height.y; ++Y)
            {
                for (int16_t X = Width.x; X < Width.y; ++X)
                {
                    // Windows expects BGR.
                    const auto Offset{ (Y - Area.y0) % Imageheight * Imagewidth + (X - Area.x0) };
                    Internal::setPixel({ X, Y }, { Pixels[Offset].RGBA.B, Pixels[Offset].RGBA.G, Pixels[Offset].RGBA.R, Pixels[Offset].RGBA.A });
                }
            }
        }

        stbi_image_free(Image);
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
    Draw::Circle(texture_t(), point2_t(), float());
    Draw::Circle(rgba_t(), point2_t(), float());
    Draw::Polygon(texture_t(), std::vector<vec2_t>());
    Draw::Polygon(rgba_t(), std::vector<vec2_t>());
    Draw::Quad(texture_t(), point4_t());
    Draw::Quad(rgba_t(), point4_t());

    Draw::Circle<true>(texture_t(), point2_t(), float());
    Draw::Circle<true>(rgba_t(), point2_t(), float());
    Draw::Polygon<true>(texture_t(), std::vector<vec2_t>());
    Draw::Polygon<true>(rgba_t(), std::vector<vec2_t>());
    Draw::Quad<true>(texture_t(), point4_t());
    Draw::Quad<true>(rgba_t(), point4_t());
}
