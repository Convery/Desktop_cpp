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
    point2_t gRenderingresolution{1280, 720};
    namespace Rendering
    {
        point4_t Currentclippingarea{};
        point4_t Globalclippingarea{};
        BITMAPINFO Format{};
        pixel24_t *Canvas{};

        // Create and invalidate part of a framebuffer.
        void Createframebuffer()
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
            Canvas = (pixel24_t *)HeapAlloc(GetProcessHeap(), NULL, (gRenderingresolution.y / 4) * (gRenderingresolution.x / 2) * sizeof(pixel24_t));

            // C-style cleanup needed.
            DeleteDC(Devicecontext);
        }
        void Invalidatearea(point4_t Area)
        {
            Globalclippingarea.x0 = std::min(Globalclippingarea.x0, Area.x0);
            Globalclippingarea.y0 = std::min(Globalclippingarea.y0, Area.y0);
            Globalclippingarea.x1 = std::max(Globalclippingarea.x1, Area.x1);
            Globalclippingarea.y1 = std::max(Globalclippingarea.y1, Area.y1);
        }

        // Callback on when to process elements.
        void onRender(const void *Context)
        {
            auto doRender = [&](point4_t Clippingarea) -> void
            {
                std::function<void(Element_t *This)> Lambda = [&](Element_t *This) -> void
                {
                    if(This->onRender) This->onRender(This);
                    for (const auto &Item : This->Children) Lambda(Item);
                };

                // Clear the clippingarea.
                Currentclippingarea = Clippingarea;
                Draw::Quad({ 0xFF, 0xFF, 0xFF, 1.0f }, Currentclippingarea);

                // Render all elements.
                assert(gRootelement);
                Lambda(gRootelement);
            };

            // Present each quadrant.
            for (int16_t i = 0; i < 4; ++i)
            {
                // Left.
                doRender(
                    {
                        std::clamp(Globalclippingarea.x0, int16_t(0),                               int16_t(gRenderingresolution.x / 2)),
                        std::clamp(Globalclippingarea.y0, int16_t(gRenderingresolution.y / 4 * i),  int16_t(gRenderingresolution.y / 4 * (i + 1))),
                        std::clamp(Globalclippingarea.x1, int16_t(0),                               int16_t(gRenderingresolution.x / 2)),
                        std::clamp(Globalclippingarea.y1, int16_t(gRenderingresolution.y / 4 * i),  int16_t(gRenderingresolution.y / 4 * (i + 1)))
                    });
                StretchDIBits(HDC(Context), 0, (gWindowsize.y / 4) * i, gWindowsize.x / 2, gWindowsize.y / 4,
                    0, 0, gRenderingresolution.x / 2, gRenderingresolution.y / 4, Canvas, &Format, DIB_RGB_COLORS, SRCCOPY);

                // Right.
                doRender(
                    {
                        std::clamp(Globalclippingarea.x0, int16_t(gRenderingresolution.x / 2),      int16_t(gRenderingresolution.x)),
                        std::clamp(Globalclippingarea.y0, int16_t(gRenderingresolution.y / 4 * i),  int16_t(gRenderingresolution.y / 4 * (i + 1))),
                        std::clamp(Globalclippingarea.x1, int16_t(gRenderingresolution.x / 2),      int16_t(gRenderingresolution.x)),
                        std::clamp(Globalclippingarea.y1, int16_t(gRenderingresolution.y / 4 * i),  int16_t(gRenderingresolution.y / 4 * (i + 1)))
                    });
                StretchDIBits(HDC(Context), gWindowsize.x / 2, (gWindowsize.y / 4) * i, gWindowsize.x / 2, gWindowsize.y / 4,
                    0, 0, gRenderingresolution.x / 2, gRenderingresolution.y / 4, Canvas, &Format, DIB_RGB_COLORS, SRCCOPY);
            }
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
                            /*
                                HACK(Convery):
                                We can not draw multiple lines with the same Y coord
                                because it messes up the alpha-blending and creates
                                an orb-like effect. https://i.imgur.com/AhdOsqp.png
                            */
                            thread_local int16_t PreviousY = -1;
                            if (PreviousY != Size.y)
                            {
                                for (int16_t X = int16_t(Origin.x - Size.x); X < int16_t(Origin.x + Size.x); ++X)
                                {
                                    doCallback({ X, int16_t(Origin.y + Size.y) });
                                    doCallback({ X, int16_t(Origin.y - Size.y) });
                                }
                                PreviousY = Size.y;
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

                // Draw the cardinals..
                doDrawing(Position, { X, Y });

                // Draw the rest of the owl..
                X++; Y = int16_t(std::sqrt(Radius * Radius - 1) + 0.5f);
                while (X < Y)
                {
                    doDrawing(Position, { X, Y });
                    X++; Y = int16_t(std::sqrt(Radius * Radius - X * X) + 0.5f);
                }

                // Precision..
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
                while (Position.y > gRenderingresolution.y / 4 - 1) Position.y -= gRenderingresolution.y / 4;
                if (Position.x > gRenderingresolution.x / 2 - 1) Position.x -= gRenderingresolution.x / 2;
                Canvas[Position.y * (gRenderingresolution.x / 2) + Position.x] = Color;
            }
            ainline void setPixel(point2_t Position, const pixel24_t Color, const float Alpha)
            {
                if (Alpha == 1.0f) setPixel(Position, Color);
                else
                {
                    while (Position.y > gRenderingresolution.y / 4) Position.y -= gRenderingresolution.y / 4;
                    if (Position.x > gRenderingresolution.x / 2) Position.x -= gRenderingresolution.x / 2;
                    auto Base{ Canvas[Position.y * (gRenderingresolution.x / 2) + Position.x] };
                    Base.BGR.B = uint8_t(Base.BGR.B * (1.0f - Alpha) + Color.BGR.B * Alpha);
                    Base.BGR.G = uint8_t(Base.BGR.G * (1.0f - Alpha) + Color.BGR.G * Alpha);
                    Base.BGR.R = uint8_t(Base.BGR.R * (1.0f - Alpha) + Color.BGR.R * Alpha);
                    setPixel(Position, Base);
                }
            }
        }

        // Basic drawing.
        namespace Draw
        {
            template <bool Outline> void Circle(const texture_t Color, const point2_t Position, const float Radius)
            {
                if (Color.Alpha == 0.0f) return;
                if (Outline)
                {
                    Internal::outlineCircle(Position, Radius, [&](const point2_t Position) -> void
                    {
                        Internal::setPixel(Position, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x % Color.Size.x]);
                    });
                }
                else
                {
                    Internal::fillCircle(Position, Radius, [&](const point2_t Position) -> void
                    {
                        Internal::setPixel(Position, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x % Color.Size.x]);
                    });
                }
            }
            template <bool Outline> void Circle(const rgba_t Color, const point2_t Position, const float Radius)
            {
                if (Color.A == 0.0f) return;
                const auto Pixel{ Internal::fromRGBA(Color) };

                if (Outline)
                {
                    Internal::outlineCircle(Position, Radius, [&](const point2_t Position) -> void
                    {
                        Internal::setPixel(Position, Pixel, Color.A);
                    });
                }
                else
                {
                    Internal::fillCircle(Position, Radius, [&](const point2_t Position) -> void
                    {
                        Internal::setPixel(Position, Pixel, Color.A);
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
                    Internal::fillPolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position) -> void
                    {
                        Internal::setPixel(Position, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x % Color.Size.x], Color.Alpha);
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
                    Internal::fillPolygon(Vertices.data(), Vertices.size(), [&](const point2_t Position) -> void
                    {
                        Internal::setPixel(Position, Pixel, Color.A);
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
                    Internal::fillPolygon(Vertices, 4, [&](const point2_t Position) -> void
                    {
                        Internal::setPixel(Position, ((pixel24_t *)Color.Data)[(Position.y % Color.Size.y) * Color.Size.x + Position.x % Color.Size.x], Color.Alpha);
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
                    Internal::fillPolygon(Vertices, 4, [&](const point2_t Position) -> void
                    {
                        Internal::setPixel(Position, Pixel, Color.A);
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

        // CL refuses to instantiate our drawing functions so we need to reference them once.
        void Microsofthackery()
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
    }
}
#else
    #error Non-windows abstraction is not implemented (yet!)
#endif
