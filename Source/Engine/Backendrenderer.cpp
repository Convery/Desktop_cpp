/*
    Initial author: Convery (tcn@ayria.se)
    Started: 12-07-2018
    License: MIT

    Turns code into colors on the screen =P
*/

#include "../Stdinclude.hpp"

#if !defined(_WIN32)
    #error Non-windows abstraction is not implemented (yet!)
#else
namespace Engine::Rendering
{
    const point4_t Defaultclippingarea{ -1, -1, -1, -1 };
    point4_t Globalclippingarea{ Defaultclippingarea };
    point4_t Currentclippingarea{};
    BITMAPINFO Format{};
    pixel24_t *Canvas{};

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
        //assert(gRootelement);
        //Tick(gRootelement);

        // The scaling we need to do if the window isn't the same as the renderer.
        const float XMultiplier{ (float)gWindowsize.x / (float)gRenderingresolution.x };
        const float YMultiplier{ (float)gWindowsize.y / (float)gRenderingresolution.y };

        // If the global area is unmodified, don't render anything.
        if (0 == std::memcmp(Globalclippingarea.Raw, Defaultclippingarea.Raw, sizeof(point4_t))) return;

        // Render a part of the window.
        auto Renderquadrant = [&](point4_t Clippingarea) -> void
        {
            // Clear the buffer.
            Currentclippingarea = Clippingarea;
            std::memset(Canvas, 0xFF, (gRenderingresolution.y / 4) * (gRenderingresolution.x / 2) * sizeof(pixel24_t));

            // Debugging borders for the quadrants.
            if constexpr(Build::Debug::isDebugging)
            {
                Draw::Quad<true>({ 0xFF, 0, 0xFF, 1 }, Clippingarea);
            }

            // Helper to save my fingers.
            std::function<void(Element_t *)> Render = [&](Element_t *This) -> void
            {
                if (This->onFrame) This->onRender(This);
                for (const auto &Item : This->Childelements) Render(Item);
            };

            // Render all elements.
            //assert(gRootelement);
            //Render(gRootelement);
        };

        // Render each dirty quadrant.
        for (int16_t i = 0; i < 4; ++i)
        {
            // Out of bounds.
            if (Globalclippingarea.x0 > int16_t(gRenderingresolution.x / 2)) continue;
            if (Globalclippingarea.y1 < int16_t(gRenderingresolution.y / 4 * i)) continue;
            if (Globalclippingarea.y0 > int16_t(gRenderingresolution.y / 4 * (i + 1))) continue;
            const point4_t Rect { 0, int16_t(gRenderingresolution.y / 4 * i), int16_t(gRenderingresolution.x / 2 - 1), int16_t(gRenderingresolution.y / 4 * (i + 1) - 1) };

            Renderquadrant(Rect);
            StretchDIBits(HDC(Context), int(Rect.x0 * XMultiplier), int(Rect.y0 * YMultiplier), int(gRenderingresolution.x / 2 * XMultiplier),
                int(gRenderingresolution.y / 4 * YMultiplier), 0, 0, gRenderingresolution.x / 2, gRenderingresolution.y / 4,
                Canvas, &Format, DIB_RGB_COLORS, SRCCOPY);
        }
        for (int16_t i = 0; i < 4; ++i)
        {
            // Out of bounds.
            if (Globalclippingarea.x1 < int16_t(gRenderingresolution.x / 2)) continue;
            if (Globalclippingarea.y1 < int16_t(gRenderingresolution.y / 4 * i)) continue;
            if (Globalclippingarea.y0 > int16_t(gRenderingresolution.y / 4 * (i + 1))) continue;
            const point4_t Rect { int16_t(gRenderingresolution.x / 2), int16_t(gRenderingresolution.y / 4 * i), int16_t(gRenderingresolution.x - 1), int16_t(gRenderingresolution.y / 4 * (i + 1) - 1) };

            Renderquadrant(Rect);
            StretchDIBits(HDC(Context), int(Rect.x0 * XMultiplier), int(Rect.y0 * YMultiplier), int(gRenderingresolution.x / 2 * XMultiplier),
                int(gRenderingresolution.y / 4 * YMultiplier), 0, 0, gRenderingresolution.x / 2, gRenderingresolution.y / 4,
                Canvas, &Format, DIB_RGB_COLORS, SRCCOPY);
        }

        // Reset the dirty area.
        Globalclippingarea = Defaultclippingarea;
    }

    // Create the framebuffer on startup.
    struct Initializer
    {
        Initializer()
        {
            auto Devicecontext{ GetDC(NULL) };

            // Bitmap format, upside-down because Windows.
            Format.bmiHeader.biSize = sizeof(BITMAPINFO);
            Format.bmiHeader.biHeight = -(gRenderingresolution.y / 4);
            Format.bmiHeader.biWidth = (gRenderingresolution.x / 2);
            Format.bmiHeader.biCompression = BI_RGB;
            Format.bmiHeader.biBitCount = 24;
            Format.bmiHeader.biPlanes = 1;

            // Create the new canvas.
            if (Canvas) HeapFree(GetProcessHeap(), NULL, Canvas);
            Canvas = (pixel24_t *)HeapAlloc(GetProcessHeap(), NULL, (gRenderingresolution.y / 4 + 1) * (gRenderingresolution.x / 2) * sizeof(pixel24_t));

            // C-style cleanup needed.
            DeleteDC(Devicecontext);
        }
    };
    static Initializer Loader{};
}
#endif

// Drawing primitives.
namespace Engine::Rendering::Draw::Internal
{
    // Helpers to place all writes in a single place.
    ainline pixel24_t fromRGBA(const rgba_t Color)
    {
        return
        {
            uint8_t(Color.B <= 1 ? Color.B * 255 : Color.B),
            uint8_t(Color.G <= 1 ? Color.G * 255 : Color.G),
            uint8_t(Color.R <= 1 ? Color.R * 255 : Color.R)
        };
    }
    ainline void setPixel(point2_t Position, const pixel24_t Color)
    {
        Canvas[(Position.y % (gRenderingresolution.y / 4)) * (gRenderingresolution.x / 2) + Position.x % (gRenderingresolution.x / 2)] = Color;
    }
    ainline void setPixel(point2_t Position, const pixel24_t Color, const float Alpha)
    {
        if (Alpha == 1.0f) setPixel(Position, Color);
        else
        {
            auto Base{ Canvas[(Position.y % (gRenderingresolution.y / 4)) * (gRenderingresolution.x / 2) + Position.x % (gRenderingresolution.x / 2)] };
            Base.BGR.B = uint8_t(Base.BGR.B * (1.0f - Alpha) + Color.BGR.B * Alpha);
            Base.BGR.G = uint8_t(Base.BGR.G * (1.0f - Alpha) + Color.BGR.G * Alpha);
            Base.BGR.R = uint8_t(Base.BGR.R * (1.0f - Alpha) + Color.BGR.R * Alpha);
            setPixel(Position, Base);
        }
    }

    // Draw a simple line, should add some anti-aliasing later.
    template<typename CB = std::function<void(const point2_t Position)>>
    ainline void drawLine(point2_t Start, point2_t Stop, CB Callback)
    {
        auto Localarea{ Currentclippingarea };

        // Check if we should invert the line so we always draw left to right.
        const bool Steep{ std::abs(Start.x - Stop.x) < std::abs(Start.y - Stop.y) };
        if (Steep)
        {
            std::swap(Start.x, Start.y);
            std::swap(Stop.x, Stop.y);
            std::swap(Localarea.x0, Localarea.y0);
            std::swap(Localarea.x1, Localarea.y1);
        }
        if (Start.x > Stop.x || Start.y > Stop.y) std::swap(Start, Stop);

        // How much to move per iteration.
        const auto DeltaX{ int16_t(Stop.x - Start.x) };
        const auto DeltaY{ int16_t(Stop.y - Start.y) };

        // For each X coordinate.
        int16_t Y{ Start.y }, Error{};
        const auto Deltaerror{ int16_t(std::abs(DeltaY) * 2) };
        for (int16_t X = std::max(Start.x, Localarea.x0); X <= std::min(Stop.x, Localarea.x1); ++X)
        {
            // Invert the coordinates if too steep.
            if (Steep) Callback({ Y, X });
            else Callback({ X, Y });

            // Update the error offset.
            Error += Deltaerror;
            if (Error > DeltaX)
            {
                Y += Stop.y > Start.y ? 1 : -1;
                Error -= DeltaX * 2;

                // out of bounds.
                if (Y < Localarea.y0 || Y > Localarea.y1) break;
            }
        }
    }

    // The simplest drawing-call ever.
    template<typename CB = std::function<void(const point2_t Position, const size_t Length)>>
    ainline void fillRect(const point4_t Area, CB Callback)
    {
        const auto Width{ Area.x1 - Area.x0 };
        const auto Height{ Area.y1 - Area.y0 };
        for (int16_t i = 0; i < Height; ++i) Callback({ Area.x0, Area.y0 + i }, Width);
    }

    // Outline and fill polygons.
    template<typename CB = std::function<void(const point2_t Position, const size_t Length)>>
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
    template <bool Fill = true, typename CB = std::function<void(const point2_t Position, const size_t Length)>>
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
            if (Position.y < Area.y0 || Position.y > Area.y1) return;
            const point2_t Scanline{ std::max(Position.x, Area.x0), std::min(int16_t(Position.x + Length), Area.x1) };
            if (Scanline.y - Scanline.x > 1) Callback({ Scanline.x, Position.y }, Scanline.y - Scanline.x);
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
        X++; Y = int16_t(std::sqrt(R2 - int16_t(1)) + 0.5f);
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
    template<typename CB = std::function<void(const point2_t Position, const size_t Length)>>
    ainline void fillCircle(const point2_t Position, const float Radius, CB Callback)
    {
        return drawCircle<true>(Position, Radius, Callback);
    }
    template<typename CB = std::function<void(const point2_t Position, const size_t Length)>>
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
        if (Color.Alpha == 0.0f) return;
        if (Outline)
        {
            Internal::outlineCircle(Position, Radius, [&](const point2_t Position, const size_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                    Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x + i % Color.Size.x], Color.Alpha);
            });
        }
        else
        {
            Internal::fillCircle(Position, Radius, [&](const point2_t Position, const size_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                    Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x + i % Color.Size.x], Color.Alpha);
            });
        }
    }
    template <bool Outline> void Circle(const rgba_t Color, const point2_t Position, const float Radius)
    {
        if (Color.A == 0.0f) return;
        const auto Pixel{ Internal::fromRGBA(Color) };

        if (Outline)
        {
            Internal::outlineCircle(Position, Radius, [&](const point2_t Position, const size_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel, Color.A);
            });
        }
        else
        {
            Internal::fillCircle(Position, Radius, [&](const point2_t Position, const size_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel, Color.A);
            });
        }
    }
    template <bool Outline> void Polygon(const texture_t Color, const std::vector<vec2_t> Vertices)
    {
        if (Color.Alpha == 0.0f) return;
        Internal::outlinePolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x % Color.Size.x], Color.Alpha);
        });
        if (!Outline)
        {
            Internal::fillPolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position, const size_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                    Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x + i % Color.Size.x], Color.Alpha);
            });
        }
    }
    template <bool Outline> void Polygon(const rgba_t Color, const std::vector<vec2_t> Vertices)
    {
        if (Color.A == 0.0f) return;
        Internal::outlinePolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, Pixel, Color.A);
        });
        if (!Outline)
        {
            Internal::fillPolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position, const size_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel, Color.A);
            });
        }
    }
    template <bool Outline> void Quad(const texture_t Color, const point4_t Area)
    {
        if (Color.Alpha == 0.0f) return;
        const vec2_t Vertices[] = { {Area.x0, Area.y0}, {Area.x1, Area.y0}, {Area.x1, Area.y1}, {Area.x0, Area.y1}, {Area.x0, Area.y0} };

        Internal::outlinePolygon(Vertices, 5, [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x % Color.Size.x], Color.Alpha);
        });
        if (!Outline)
        {
            Internal::fillRect(Area, [&](const point2_t Position, const size_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                    Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x + i % Color.Size.x], Color.Alpha);
            });
        }
    }
    template <bool Outline> void Quad(const rgba_t Color, const point4_t Area)
    {
        if (Color.A == 0.0f) return;
        const auto Pixel{ Internal::fromRGBA(Color) };
        const vec2_t Vertices[] = { {Area.x0, Area.y0}, {Area.x1, Area.y0}, {Area.x1, Area.y1}, {Area.x0, Area.y1}, {Area.x0, Area.y0} };

        Internal::outlinePolygon(Vertices, 5, [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, Pixel, Color.A);
        });
        if (!Outline)
        {
            Internal::fillRect(Area, [&](const point2_t Position, const size_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel, Color.A);
            });
        }
    }
    void Line(const texture_t Color, const point2_t Start, const point2_t Stop)
    {
        if (Color.Alpha == 0.0f) return;
        Internal::drawLine(Start, Stop, [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x % Color.Size.x], Color.Alpha);
        });
    }
    void Line(const rgba_t Color, const point2_t Start, const point2_t Stop)
    {
        if (Color.A == 0.0f) return;
        const auto Pixel{ Internal::fromRGBA(Color) };

        Internal::drawLine(Start, Stop, [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, Pixel, Color.A);
        });
    }
}
