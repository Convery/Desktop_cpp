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
#include <future>



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
    ainline void setPixel(const size_t Offset, const pixel32_t Color)
    {
        // Alpha is always the last entry.
        if (Color.Raw[3] == 0xFF)
        {
            std::memcpy(((pixel24_t *)Scanline)[Offset].Raw, Color.Raw, sizeof(pixel24_t));
        }
        else
        {
            #define BLEND(A, B) A += int32_t((((B - A) * Color.Raw[3]))) >> 8;
            BLEND(((pixel24_t *)Scanline)[Offset].Raw[0], Color.Raw[0]);
            BLEND(((pixel24_t *)Scanline)[Offset].Raw[1], Color.Raw[1]);
            BLEND(((pixel24_t *)Scanline)[Offset].Raw[2], Color.Raw[2]);
        }
    }
    ainline void setPixel(const size_t Offset, const pixel24_t Color)
    {
        return setPixel(Offset, { Color.Raw[0], Color.Raw[1], Color.Raw[2], 0xFF });
    }

    // Outline and fill circles.
    template <bool Fill = true, typename CB = std::function<void(const point2_t Position, const int16_t Length)>>
    ainline void drawCircle(const point2_t Position, const float Radius, CB Callback)
    {
        const double R2{ Radius * Radius };
        int16_t X{}, Y{ int16_t(Radius) };

        // Helpers to keep the algorithm cleaner.
        auto doCallback = [&Callback](const point2_t Position, const size_t Length = 1) -> void
        {
            if (Position.y == Currentline)
            {
                const point2_t Scanline{ Position.x, std::min(int16_t(Position.x + Length), gWindowsize.x) };
                Callback({ Scanline.x, Position.y }, std::max(int16_t(Scanline.y - Scanline.x), int16_t(1)));
            }
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


    #if 0

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

    #endif
}

#if !defined(_WIN32)
    #error Non-windows abstraction is not implemented (yet!)
#else
namespace Engine::Rendering
{
    int16_t Currentline;
    uint8_t *Scanline;

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

        // Initialize the scanline info for this pass.
        static BITMAPINFO Lineformat{ { sizeof(BITMAPINFO), 1, 1, 1, 24 } };
        const uint32_t Width{ uint32_t(gWindowsize.x * 3) };
        Lineformat.bmiHeader.biWidth = gWindowsize.x;

        // Create a nice little buffer on the stack.
        __asm
        {
            sub esp, Width;
            mov edi, esp;
            mov Scanline, esp;
        }

        // Render all the scanlines in the main thread.
        for (int16_t i = 0; i < gWindowsize.y; ++i)
        {
            Currentline = i;
            std::memset(Scanline, 0xFF, Width);

            // Helper to save my fingers.
            std::function<void(Element_t *)> Render = [&](Element_t *This) -> void
            {
                if (This->Dimensions.y0 > Currentline || This->Dimensions.y1 < Currentline)
                    return;

                if (This->onRender) This->onRender(This);
                for (const auto &Item : This->Childelements) Render(Item);
            };

            // Render all elements.
            assert(gRootelement);
            Render(gRootelement);

            // Bitblt to screen.
            SetDIBitsToDevice((HDC)Context, 0, i, gWindowsize.x, 1, 0, 0, 0, 1, Scanline, &Lineformat, DIB_RGB_COLORS);
        }

        // Delete our buffer.
        __asm add esp, Width;
    }
}
#endif

/*
    NOTE(Convery):

    So apparently CL refuses to instantiate our templates if they are created in other modules.
    So we need to touch them once in this module so that the linker has something to poke at.
    Don't ask me why, just roll with it.
*/

void Microsoft_hackery_do_not_call_this()
{
    #if 0
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
    #endif
}
