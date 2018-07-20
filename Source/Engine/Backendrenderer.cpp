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
    point2_t gRenderingresolution{};
    point4_t Currentclippingarea{};
    BITMAPINFO Format{};
    pixel24_t *Canvas{};
    pixel24_t *Old{};

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
        auto Renderquadrant = [&](point4_t Clippingarea) -> void
        {
            Currentclippingarea = Clippingarea;
            /*
            {
                std::max(Clippingarea.x0, int16_t(0)),
                std::max(Clippingarea.y0, int16_t(0)),
                std::min(Clippingarea.x1, int16_t(gRenderingresolution.x - 1)),
                std::min(Clippingarea.y1, int16_t(gRenderingresolution.y - 1))
            };
            */

            // Clear the buffer.
            std::memset(Canvas, 0xFF, (gRenderingresolution.y / 4) * gRenderingresolution.x * sizeof(pixel24_t));

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
                //Draw::Quad<true>({ 0xFF, 0, 0xFF, 1 }, Clippingarea);
            }
        };

        // Render each dirty quadrant.
        for (int16_t i = 0; i < 4; ++i)
        {
            // Out of bounds.
            if (Globalclippingarea.x0 > int16_t(gRenderingresolution.x)) continue;
            if (Globalclippingarea.y1 < int16_t(gRenderingresolution.y / 4 * i)) continue;
            if (Globalclippingarea.y0 > int16_t(gRenderingresolution.y / 4 * (i + 1))) continue;
            const point4_t Rect { 0, int16_t(gRenderingresolution.y / 4 * i), int16_t(gRenderingresolution.x), int16_t(gRenderingresolution.y / 4 * (i + 1)) };

            Renderquadrant(Rect);
            StretchDIBits(HDC(Context), int(Rect.x0), int(Rect.y0), int(gRenderingresolution.x), int(gRenderingresolution.y / 4),
                0, 0, gRenderingresolution.x, gRenderingresolution.y / 4, Canvas, &Format, DIB_RGB_COLORS, SRCCOPY);
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
        Format.bmiHeader.biSize = sizeof(BITMAPINFO);
        Format.bmiHeader.biHeight = -(gRenderingresolution.y / 4);
        Format.bmiHeader.biWidth = gRenderingresolution.x;
        Format.bmiHeader.biCompression = BI_RGB;
        Format.bmiHeader.biBitCount = 24;
        Format.bmiHeader.biPlanes = 1;

        // Create the new canvas.
        if (Old) HeapFree(GetProcessHeap(), NULL, Old);
        if (Canvas) Old = Canvas;
        Canvas = (pixel24_t *)HeapAlloc(GetProcessHeap(), NULL, (gRenderingresolution.y / 4 + 1) * (gRenderingresolution.x) * sizeof(pixel24_t));
        std::thread([]() {std::this_thread::sleep_for(std::chrono::seconds(1)); if (Old) HeapFree(GetProcessHeap(), NULL, Old);  }).detach();

        // C-style cleanup needed.
        DeleteDC(Devicecontext);
    }
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
    ainline void setPixel(const point2_t Position, const pixel24_t Color)
    {
        Canvas[Position.y * gRenderingresolution.x + Position.x] = Color;
    }
    ainline void setPixel(const point2_t Position, const pixel24_t Color, const float Alpha)
    {
        if (Alpha == 1.0f) setPixel(Position, Color);
        else
        {
            auto Base{ Canvas[Position.y * gRenderingresolution.x + Position.x] };
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
        for (int16_t X = Start.x; X < std::min(Stop.x, Localarea.x1); X++)
        {
            // End of line.
            if (Y > Localarea.y1) break;

            // Inside the clipping area.
            if (Y >= Localarea.y0 && X >= Localarea.x0)
            {
                // Invert the coordinates if too steep.
                if (Steep) Callback({ Y, X % (gRenderingresolution.y / 4) });
                else Callback({ X, Y % (gRenderingresolution.y / 4) });
            }

            // Update the error offset.
            Error += Deltaerror;
            if (Error > DeltaX)
            {
                Y += Stop.y > Start.y ? 1 : -1;
                Error -= DeltaX * 2;
            }
        }
    }

    // The simplest drawing-call ever.
    template<typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void fillRect(const point4_t Area, CB Callback)
    {
        const int16_t Width{ std::min(Area.x1, Currentclippingarea.x1) - std::max(Area.x0, Currentclippingarea.x0) };
        for (int16_t Y = std::max(Area.y0, Currentclippingarea.y0); Y < std::min(Area.y1, Currentclippingarea.y1); ++Y)
            Callback({ std::max(Area.x0, Currentclippingarea.x0), Y % (gRenderingresolution.y / 4) }, Width);
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

                    Callback({ Nodes[i], Y % (gRenderingresolution.y / 4) }, Nodes[i + 1] - Nodes[i] + 1);
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
            Callback({ Scanline.x, Position.y % (gRenderingresolution.y / 4) }, std::max(int16_t(Scanline.y - Scanline.x), int16_t(1)));
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
        if (Color.Alpha == 0.0f) return;
        if (Outline)
        {
            Internal::outlineCircle(Position, Radius, [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                    Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x + i % Color.Size.x], Color.Alpha);
            });
        }
        else
        {
            Internal::fillCircle(Position, Radius, [&](const point2_t Position, const int16_t Length) -> void
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
            Internal::outlineCircle(Position, Radius, [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel, Color.A);
            });
        }
        else
        {
            Internal::fillCircle(Position, Radius, [&](const point2_t Position, const int16_t Length) -> void
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
            Internal::fillPolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i)
                    Internal::setPixel({ Position.x + i, Position.y }, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x + i % Color.Size.x], Color.Alpha);
            });
        }
    }
    template <bool Outline> void Polygon(const rgba_t Color, const std::vector<vec2_t> Vertices)
    {
        if (Color.A == 0.0f) return;
        const auto Pixel{ Internal::fromRGBA(Color) };

        Internal::outlinePolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, Pixel, Color.A);
        });
        if (!Outline)
        {
            Internal::fillPolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position, const int16_t Length) -> void
            {
                for (int16_t i = 0; i < Length; ++i) Internal::setPixel({ Position.x + i, Position.y }, Pixel, Color.A);
            });
        }
    }
    template <bool Outline> void Quad(const texture_t Color, const point4_t Area)
    {
        if (Color.Alpha == 0.0f) return;
        #pragma warning(suppress: 4838)
        const vec2_t Vertices[] =
        {
            {std::max(Area.x0, Currentclippingarea.x0), std::max(Area.y0, Currentclippingarea.y0)},
            {std::min(Area.x1, int16_t(Currentclippingarea.x1 - 1)), std::max(Area.y0, Currentclippingarea.y0)},
            {std::min(Area.x1, int16_t(Currentclippingarea.x1 - 1)), std::min(Area.y1, int16_t(Currentclippingarea.y1 - 1))},
            {std::max(Area.x0, Currentclippingarea.x0), std::min(Area.y1, int16_t(Currentclippingarea.y1 - 1))},
            {std::max(Area.x0, Currentclippingarea.x0), std::max(Area.y0, Currentclippingarea.y0)}
        };

        Internal::outlinePolygon(Vertices, 5, [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x % Color.Size.x], Color.Alpha);
        });
        if (!Outline)
        {
            Internal::fillRect(Area, [&](const point2_t Position, const int16_t Length) -> void
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
        #pragma warning(suppress: 4838)
        const vec2_t Vertices[] =
        {
            {std::max(Area.x0, Currentclippingarea.x0), std::max(Area.y0, Currentclippingarea.y0)},
            {std::min(Area.x1, int16_t(Currentclippingarea.x1 - 1)), std::max(Area.y0, Currentclippingarea.y0)},
            {std::min(Area.x1, int16_t(Currentclippingarea.x1 - 1)), std::min(Area.y1, int16_t(Currentclippingarea.y1 - 1))},
            {std::max(Area.x0, Currentclippingarea.x0), std::min(Area.y1, int16_t(Currentclippingarea.y1 - 1))},
            {std::max(Area.x0, Currentclippingarea.x0), std::max(Area.y0, Currentclippingarea.y0)}
        };

        Internal::outlinePolygon(Vertices, 5, [&](const point2_t Position) -> void
        {
            Internal::setPixel(Position, Pixel, Color.A);
        });
        if (!Outline)
        {
            Internal::fillRect(Area, [&](const point2_t Position, const int16_t Length) -> void
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

    // Special drawing.
    void Pixelmask(const rgba_t Color, const point2_t Size, const size_t Segmentcount, const Pixelmask::segment_t *Segments)
    {
        const auto Pixel{ Internal::fromRGBA(Color) };
        size_t Iterator = 0;

        for (size_t i = 0; i < Segmentcount; ++i)
        {
            auto Filled{ Pixelmask::Filled(Segments[i]) };
            auto Blank{ Pixelmask::Blank(Segments[i]) };

            Iterator += Blank;
            for (size_t c = 0; c < Filled; c++)
            {
                const auto Y = Iterator / Size.x;
                const auto X = Iterator % Size.x;

                if (Y >= Currentclippingarea.y0 && Y <= Currentclippingarea.y1)
                {
                    if (X >= Currentclippingarea.x0 && X <= Currentclippingarea.x1)
                    {
                        Internal::setPixel(
                            {
                                int16_t(X % gRenderingresolution.x),
                                int16_t(Y % (gRenderingresolution.y / 4))
                            }, Pixel);
                    }
                }

                Iterator++;
            }
        }
    }
}

/*
    NOTE(Convery):

    So apparently CL refuses to instantiate our templates if they are created in other objects.
    So we need to touch them once in this object so that the linker has something to poke at.
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
