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
            // Fill a polygon.
            ainline void fillPolygon(const vec2_t *Vertices, const size_t Verticecount, std::function<void(const point2_t Position)> Callback)
            {
                auto Nodes{ std::make_unique<uint16_t[]>(Verticecount + 1) };

                // For each row in the canvas.
                for (uint16_t Y = Currentclippingarea.y0; Y < Currentclippingarea.y1; ++Y)
                {
                    uint16_t i{}, j{ uint16_t(Verticecount - 1) }, Nodecount{};

                    // Create a few nodes.
                    for (i = 0; i < Verticecount; ++i)
                    {
                        if ((Vertices[i].y <= Y && Vertices[j].y >= Y) || (Vertices[j].y <= Y && Vertices[i].y >= Y))
                        {
                            if (Vertices[j].y - Vertices[i].y == 0) Nodes[Nodecount++] = Vertices[i].x + (Y - Vertices[i].y) * (Vertices[j].x - Vertices[i].x);
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

            // Draw a line.
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
                const auto DeltaX{ Stop.x - Start.x };
                const auto DeltaY{ Stop.y - Start.y };

                // For each X coordinate.
                uint16_t Y{ Start.y }, Error{};
                const auto Deltaerror{ std::abs(DeltaY) * 2 };
                for (uint16_t X = std::max(Start.x, Currentclippingarea.x0); X <= std::min(Stop.x, Currentclippingarea.x1); ++X)
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
            ainline void drawOutline(const vec2_t *Vertices, const size_t Verticecount, std::function<void(const point2_t Position)> Callback)
            {
                for (size_t i = 0; i < Verticecount - 1; i++)
                {
                    drawLine({ uint16_t(Vertices[i].x), uint16_t(Vertices[i].y) }, { uint16_t(Vertices[i + 1].x), uint16_t(Vertices[i + 1].y) }, Callback);
                }
            }
        }
    }
}
#else
    #error Non-windows abstraction is not implemented (yet!)
#endif
