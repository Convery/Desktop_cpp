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

    /*
        TODO(tcn):
        We need to build a graph over the elements.
        So that we don't re-parse the JSON every frame.
    */
    std::vector<void (*)(const vec4_t Viewport)> Drawcalls{};
    void Renderframe(const vec4_t Viewport)
    {
        for (const auto &Item : Drawcalls) Item(Viewport);
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

    namespace Gradient
    {
        void Outlinepolygon(std::vector<vec2_t> &&Points, rgba_t Color1, rgba_t Color2, uint32_t Steps);
        void Fillpolygon(std::vector<vec2_t> &&Points, rgba_t Color1, rgba_t Color2, uint32_t Steps);
        void Line(vec2_t Start, vec2_t Stop, rgba_t Color1, rgba_t Color2, uint32_t Steps);
        void Outlinerectangle(vec4_t Region, rgba_t Color1, rgba_t Color2, uint32_t Steps);
        void Fillrectangle(vec4_t Region, rgba_t Color1, rgba_t Color2, uint32_t Steps);
    }

    namespace Textured
    {
        void Outlinepolygon(std::vector<vec2_t> &&Points, vec2_t Anchor, Texture32_t Texture);
        void Fillpolygon(std::vector<vec2_t> &&Points, vec2_t Anchor, Texture32_t Texture);
        void Line(vec2_t Start, vec2_t Stop, vec2_t Anchor, Texture32_t Texture);
        void Outlinerectangle(vec4_t Region, vec2_t Anchor, Texture32_t Texture);
        void Fillrectangle(vec4_t Region, vec2_t Anchor, Texture32_t Texture);
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
}
