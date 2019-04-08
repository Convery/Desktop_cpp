/*
    Initial author: Convery (tcn@ayria.se)
    Started: 22-03-2019
    License: MIT
*/

#include "../Stdinclude.hpp"

// Wrappers for converting code to pretty colors.
namespace Rendering
{
    // Simple conversion, should be optimized.
    inline Gdiplus::Color fromRGBA(const rgba_t Color)
    {
        return
        {
            uint8_t(Color.A <= 1 ? Color.A * 255 : Color.A),
            uint8_t(Color.R <= 1 ? Color.R * 255 : Color.R),
            uint8_t(Color.G <= 1 ? Color.G * 255 : Color.G),
            uint8_t(Color.B <= 1 ? Color.B * 255 : Color.B)
        };
    }
    inline pixel32_t toPixel(const rgba_t Color)
    {
        return
        {
            uint8_t(Color.B <= 1 ? Color.B * 255 : Color.B),
            uint8_t(Color.G <= 1 ? Color.G * 255 : Color.G),
            uint8_t(Color.R <= 1 ? Color.R * 255 : Color.R),
            uint8_t(Color.A <= 1 ? Color.A * 255 : Color.A)
        };
    }

    /*
        TODO(tcn):
        We need to build a graph over the elements.
        So that we don't re-parse the JSON every frame.
    */
    std::vector<void (*)()> Drawcalls{};
    void Renderframe()
    {
        for (const auto &Item : Drawcalls) Item();
    }
    void Buildcallgraph(Element_t *Node)
    {
        if (Node->onRender) Drawcalls.push_back(Node->onRender);
        for (const auto &Child : Node->Children) Buildcallgraph(Child.get());
    }
    void Clearcallgraph()
    {
        Drawcalls.clear();
    }

    namespace Solid
    {
        void Outlinepolygon(std::vector<vec2_t> &&Points, rgba_t Color);
        void Fillpolygon(std::vector<vec2_t> &&Points, rgba_t Color);
        void Line(vec2_t Start, vec2_t Stop, rgba_t Color)
        {
            // Not allowed to be initialized without a color.. Silly..
            static auto Pen{ new Gdiplus::Pen(Gdiplus::Color()) };
            Pen->SetColor(fromRGBA(Color));

            Global.Drawingcontext->DrawLine(Pen, Start.x, Start.y, Stop.x, Stop.y);
        }
        void Outlinerectangle(vec4_t Region, rgba_t Color)
        {
            // Not allowed to be initialized without a color.. Silly..
            static auto Pen{ new Gdiplus::Pen(Gdiplus::Color()) };
            Pen->SetColor(fromRGBA(Color));

            // C-style array needed.
            std::array<Gdiplus::PointF, 5> Points
            {
                {
                    { Region.x0, Region.y0 },
                    { Region.x1, Region.y0 },
                    { Region.x1, Region.y1 },
                    { Region.x0, Region.y1 },
                    { Region.x0, Region.y0 }
                }
            };

            Global.Drawingcontext->DrawLines(Pen, Points.data(), 5);
        }
        void Fillrectangle(vec4_t Region, rgba_t Color)
        {
            // Not allowed to be initialized without a color.. Silly..
            static auto Brush{ new Gdiplus::SolidBrush(Gdiplus::Color()) };
            Brush->SetColor(fromRGBA(Color));

            Global.Drawingcontext->FillRectangle(Brush, Region.x0, Region.y0, Region.x1 - Region.x0, Region.y1 - Region.y0);
        }
    }

    void Drawimage(vec2_t Position, vec2_t Size, void *Pixels, vec2_t Offset)
    {
        BITMAPINFO BMI{ sizeof(BITMAPINFO), Size.x, -Size.y, 1, 32 };
        auto Devicecontext = Global.Drawingcontext->GetHDC();
        auto Memory = CreateCompatibleDC(Devicecontext);
        void *Backbuffer;

        // World to screen and bumping..
        Position.x -= Global.Windowposition.x;
        Position.y -= Global.Windowposition.y;
        Position.x += Offset.x;
        Position.y += Offset.y;

        // Allocate memory for the region we want to draw.
        auto Bitmap = CreateDIBSection(Memory, &BMI, DIB_RGB_COLORS, &Backbuffer, NULL, NULL);
        SelectObject(Memory, Bitmap);

        // Take a copy of the backbuffer.
        BitBlt(Memory, 0, 0, Size.x, Size.y, Devicecontext, Position.x, Position.y, SRCCOPY);

        // Blend with the input.
        for (size_t i = 0; i < Size.x * Size.y; ++i)
        {
            const auto Color = ((pixel32_t *)Pixels)[i];

            #define BLEND(A, B) A += int32_t((((B - A) * Color.Raw[3]))) >> 8;
            BLEND(((pixel32_t *)Backbuffer)[i].Raw[0], Color.Raw[0]);
            BLEND(((pixel32_t *)Backbuffer)[i].Raw[1], Color.Raw[1]);
            BLEND(((pixel32_t *)Backbuffer)[i].Raw[2], Color.Raw[2]);
        }

        // Return the modified backbuffer.
        BitBlt(Devicecontext, Position.x, Position.y, Size.x, Size.y, Memory, 0, 0, SRCCOPY);
        Global.Drawingcontext->ReleaseHDC(Devicecontext);
        DeleteObject(Bitmap);
        DeleteDC(Memory);
    }
    Texture32_t Creategradient(rgba_t Color1, rgba_t Color2, uint32_t Steps)
    {
        pixel32_t pColor1{ toPixel(Color1) }, pColor2{ toPixel(Color2) };
        Texture32_t Result{ {Steps, 1}, new pixel32_t[Steps] };
        size_t Count = 0;

        for (double i = 0; i < 1; i += (1.0 / (Steps / 2)))
        {
            rgba_t Blended;
            Blended.R = (pColor1.BGRA.R * i) + (pColor2.BGRA.R * (1 - i));
            Blended.G = (pColor1.BGRA.G * i) + (pColor2.BGRA.G * (1 - i));
            Blended.B = (pColor1.BGRA.B * i) + (pColor2.BGRA.B * (1 - i));
            Blended.A = 1;

            Result.Data[Count++] = toPixel(Blended);
        }
        for (double i = 0; i < 1; i += (1.0 / (Steps / 2)))
        {
            rgba_t Blended;
            Blended.R = (pColor2.BGRA.R * i) + (pColor1.BGRA.R * (1 - i));
            Blended.G = (pColor2.BGRA.G * i) + (pColor1.BGRA.G * (1 - i));
            Blended.B = (pColor2.BGRA.B * i) + (pColor1.BGRA.B * (1 - i));
            Blended.A = 1;

            Result.Data[Count++] = toPixel(Blended);
        }

        return Result;
    };
}
