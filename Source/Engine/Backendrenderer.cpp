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
    point4_t Currentclippingarea{};
    point4_t Globalclippingarea{};
    BITMAPINFO Format{};
    pixel24_t *Canvas{};

    // Invalidate the area that needs to be redrawn.
    void Invalidatearea(point4_t Area)
    {
        Globalclippingarea.x0 = std::min(Globalclippingarea.x0, Area.x0);
        Globalclippingarea.y0 = std::min(Globalclippingarea.y0, Area.y0);
        Globalclippingarea.x1 = std::max(Globalclippingarea.x1, Area.x1);
        Globalclippingarea.y1 = std::max(Globalclippingarea.y1, Area.y1);
    }

    // Process elements, render, and present to the context.
    void onRender(const void *Context)
    {
        // Track the deltatime, should be less than 33ms.
        static auto Lastframe{ std::chrono::high_resolution_clock::now() };
        const auto Currentframe{ std::chrono::high_resolution_clock::now() };
        const auto Deltatime{ std::chrono::duration<double>(Currentframe - Lastframe).count() };
        Lastframe = Currentframe;

        // Upscale or downscale the coordinates.
        const float XMultiplier = gRenderingresolution.x / gWindowsize.x;
        const float YMultiplier = gRenderingresolution.y / gWindowsize.y;
        auto doRender = [&](point4_t Clippingarea) -> void
        {
            std::function<void(Element_t *This)> Lambda = [&](Element_t *This) -> void
            {
                if (This->onFrame) This->onFrame(This, Deltatime);
                for (const auto &Item : This->Childelements) Lambda(Item);
            };

            // Clear the framebuffer.
            Currentclippingarea = Clippingarea;
            std::memset(Canvas, 0xFF, (gRenderingresolution.y / 4) * (gRenderingresolution.x / 2) * sizeof(pixel24_t));

            // Render all elements.
            //assert(gRootelement);
            //Lambda(gRootelement);
        };

        // Present each quadrant.
        for (int16_t i = 0; i < 4; ++i)
        {
            const point4_t Leftrect
            {
                std::clamp(Globalclippingarea.x0, int16_t(0),                               int16_t(gRenderingresolution.x / 2)),
                std::clamp(Globalclippingarea.y0, int16_t(gRenderingresolution.y / 4 * i),  int16_t(gRenderingresolution.y / 4 * (i + 1))),
                std::clamp(Globalclippingarea.x1, int16_t(0),                               int16_t(gRenderingresolution.x / 2)),
                std::clamp(Globalclippingarea.y1, int16_t(gRenderingresolution.y / 4 * i),  int16_t(gRenderingresolution.y / 4 * (i + 1)))
            };
            const point4_t Rightrect
            {
                std::clamp(Globalclippingarea.x0, int16_t(gRenderingresolution.x / 2),      int16_t(gRenderingresolution.x)),
                std::clamp(Globalclippingarea.y0, int16_t(gRenderingresolution.y / 4 * i),  int16_t(gRenderingresolution.y / 4 * (i + 1))),
                std::clamp(Globalclippingarea.x1, int16_t(gRenderingresolution.x / 2),      int16_t(gRenderingresolution.x)),
                std::clamp(Globalclippingarea.y1, int16_t(gRenderingresolution.y / 4 * i),  int16_t(gRenderingresolution.y / 4 * (i + 1)))
            };

            doRender(Leftrect);
            StretchDIBits(HDC(Context), Leftrect.x0 * XMultiplier, Leftrect.y0 * YMultiplier, (Leftrect.x1 - Leftrect.x0) * XMultiplier,
                (Leftrect.y1 - Leftrect.y0) * YMultiplier,  0, 0, gRenderingresolution.x / 2, gRenderingresolution.y / 4,
                Canvas, &Format, DIB_RGB_COLORS, SRCCOPY);

            doRender(Rightrect);
            StretchDIBits(HDC(Context), Rightrect.x0 * XMultiplier, Rightrect.y0 * YMultiplier, (Rightrect.x1 - Rightrect.x0) * XMultiplier,
                (Rightrect.y1 - Rightrect.y0) * YMultiplier, 0, 0, gRenderingresolution.x / 2, gRenderingresolution.y / 4,
                Canvas, &Format, DIB_RGB_COLORS, SRCCOPY);
        }

        // Reset the dirty area.
        Globalclippingarea = {};
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
            Canvas = (pixel24_t *)HeapAlloc(GetProcessHeap(), NULL, (gRenderingresolution.y / 4) * (gRenderingresolution.x / 2) * sizeof(pixel24_t));

            #if !defined(NDEBUG)
            std::memset(Canvas, 0xCC, (gRenderingresolution.y / 4) * (gRenderingresolution.x / 2) * sizeof(pixel24_t));
            #endif

            // C-style cleanup needed.
            DeleteDC(Devicecontext);
        }
    };
    static Initializer Loader{};
}

#endif
