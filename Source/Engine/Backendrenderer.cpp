/*
    Initial author: Convery (tcn@ayria.se)
    Started: 26-06-2018
    License: MIT

    Provides a way to render objects into the framebuffer.
*/

#include "../Stdinclude.hpp"

#if defined(_WIN32)
namespace Engine
{
    // Get the compositions to the screen.
    namespace Rendering
    {
        std::atomic<point4_t *> Clippingarea{};
        point4_t Currentclippingarea;
        HDC Surfacecontext{};
        pixel24_t *Canvas{};
        bool Presented{};

        // Create and invalidate part of a framebuffer.
        void Createframebuffer(point2_t Size)
        {
            BITMAPINFO Format{};
            static HBITMAP Surface{};
            auto Devicecontext{ GetDC(NULL) };

            // Bitmap format, upside-down because Windows.
            Format.bmiHeader.biSize = sizeof(BITMAPINFO);
            Format.bmiHeader.biHeight = -(Size.y + 1);
            Format.bmiHeader.biCompression = BI_RGB;
            Format.bmiHeader.biWidth = Size.x;
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

            // Invalidate the framebuffer.
            Invalidatearea({ 0, 0, Size.x, Size.y });
        }
        void Invalidatearea(point4_t Area)
        {
            point4_t *Currentarea;
            point4_t *Newarea{ new point4_t };

            do
            {
                Currentarea = Clippingarea.load();
                if (!Currentarea) *Newarea = Area;
                else
                {
                    Newarea->x0 = std::min(Currentarea->x0, Area.x0);
                    Newarea->y0 = std::min(Currentarea->y0, Area.y0);
                    Newarea->x1 = std::max(Currentarea->x1, Area.x1);
                    Newarea->y1 = std::max(Currentarea->y1, Area.y1);
                }
            } while (!Clippingarea.compare_exchange_strong(Currentarea, Newarea));

            delete Currentarea;
        }

        // Callback on when to process elements.
        void onPresent(const void *Context)
        {
            StretchBlt(HDC(Context), 0, 0, gWindowsize.x, gWindowsize.y, Surfacecontext, 0, 0, gRenderingresolution.x, gRenderingresolution.y, SRCCOPY);
        }
        void onRender()
        {
            // The 'clean' area is intentionally inverted.
            point4_t *Clean = new point4_t{ gRenderingresolution.x, gRenderingresolution.y, 0, 0 };
            point4_t *Clipped;

            // Exchange the clipped area with a clean one.
            do { Clipped = Clippingarea.load(); } while (!Clippingarea.compare_exchange_strong(Clipped, Clean));

            // Set the clipping area before rendering.
            Currentclippingarea = *Clipped;
            delete Clipped;

            /*
                Do rendering.
            */
        }

        // Primitives.
        namespace Internal
        {
            // Draw a line. // TODO, some anti-aliasing.
            ainline void drawLine(point2_t Start, point2_t Stop, std::function<void(const point2_t Position)> Callback)
            {
                // Check if we should invert the line so we always draw left to right.
                const bool Steep{ std::abs(Start.x - Stop.x) < std::abs(Start.y - Stop.y) };
                if (Steep)
                {
                    std::swap(Start.x, Start.y);
                    std::swap(Stop.x, Stop.y);
                }
                if (Start.x > Stop.x || Start.y > Stop.y) std::swap(Start, Stop);

                // How much to move per iteration.
                const auto DeltaX{ int16_t(Stop.x - Start.x) };
                const auto DeltaY{ int16_t(Stop.y - Start.y) };

                // For each X coordinate.
                int16_t Y{ Start.y }, Error{};
                const auto Deltaerror{ int16_t(std::abs(DeltaY) * 2) };
                for (int16_t X = std::max(Start.x, Currentclippingarea.x0); X <= std::min(Stop.x, Currentclippingarea.x1); ++X)
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
                        if (Y < Currentclippingarea.y0 || Y > Currentclippingarea.y1) break;
                    }
                }
            }

            // Outline and fill polygons. fill -> outline
            ainline void fillPolygon(const vec2_t *Vertices, const size_t Verticecount, std::function<void(const point2_t Position)> Callback)
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

                            for (j = Nodes[i]; j <= Nodes[i + 1]; ++j)
                            {
                                Callback({ j, Y });
                            }
                        }
                    }
                }
            }
            ainline void outlinePolygon(const vec2_t *Vertices, const size_t Verticecount, std::function<void(const point2_t Position)> Callback)
            {
                for (size_t i = 0; i < Verticecount - 1; i++)
                {
                    drawLine({ int16_t(Vertices[i].x), int16_t(Vertices[i].y) }, { int16_t(Vertices[i + 1].x), int16_t(Vertices[i + 1].y) }, Callback);
                }
            }

            // Outline and fill circles.
            template <bool Fill = true>
            ainline void drawCircle(const point2_t Position, const float Radius, std::function<void(const point2_t Position)> Callback)
            {
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
                auto doCallback = [&Area, &Callback](const point2_t Position) -> void
                {
                    // Outside of the clipping area.
                    if (Position.y < Area.y0 || Position.y > Area.y1) return;
                    if (Position.x < Area.x0 || Position.x > Area.x1) return;
                    Callback(Position);
                };
                auto doDrawing = [&](const point2_t Origin, const point2_t Size) -> void
                {
                    if (Size.x == 0)
                    {
                        doCallback({ Origin.x, int16_t(Origin.y + Size.y) });
                        doCallback({ Origin.x, int16_t(Origin.y - Size.y) });

                        if (Fill)
                        {
                            for (int16_t X = int16_t(Origin.x - Size.y); X < int16_t(Origin.x + Size.y); ++X)
                                doCallback({ X, Origin.y });
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
                            for (int16_t X = int16_t(Origin.x - Size.x); X < int16_t(Origin.x + Size.x); ++X)
                            {
                                doCallback({ X, int16_t(Origin.y + Size.y) });
                                doCallback({ X, int16_t(Origin.y - Size.y) });
                            }
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
                            for (int16_t X = int16_t(Origin.x - Size.x); X < int16_t(Origin.x + Size.x); ++X)
                            {
                                doCallback({ X, int16_t(Origin.y + Size.y) });
                                doCallback({ X, int16_t(Origin.y - Size.y) });
                            }
                            for (int16_t X = int16_t(Origin.x - Size.y); X < int16_t(Origin.x + Size.y); ++X)
                            {
                                doCallback({ X, int16_t(Origin.y + Size.x) });
                                doCallback({ X, int16_t(Origin.y - Size.x) });
                            }
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

                // Draw the cardinals.
                doDrawing(Position, { X, Y });

                // Draw the rest of the owl.
                X++; Y = int16_t(std::sqrt(Radius * Radius - 1) + 0.5);
                while (X < Y)
                {
                    doDrawing(Position, { X, Y });
                    X++; Y = int16_t(std::sqrt(Radius * Radius - X * X) + 0.5);
                }
                if (X == Y)
                {
                    doDrawing(Position, { X, Y });
                }
            }
            ainline void fillCircle(const point2_t Position, const float Radius, std::function<void(const point2_t Position)> Callback)
            {
                return drawCircle<true>(Position, Radius, Callback);
            }
            ainline void outlineCircle(const point2_t Position, const float Radius, std::function<void(const point2_t Position)> Callback)
            {
                return drawCircle<false>(Position, Radius, Callback);
            }
        }
    }
}
#else
    #error Non-windows abstraction is not implemented (yet!)
#endif
