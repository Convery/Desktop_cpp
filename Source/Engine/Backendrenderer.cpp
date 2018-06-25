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
        }
        void Invalidatearea(point4_t Area)
        {
            point4_t *Currentarea;
            point4_t *Newarea{ new point4_t };

            do
            {
                Currentarea = Clippingarea.load();
                Newarea->x0 = std::min(Currentarea->x0, Area.x0);
                Newarea->y0 = std::min(Currentarea->y0, Area.y0);
                Newarea->x1 = std::max(Currentarea->x1, Area.x1);
                Newarea->y1 = std::max(Currentarea->y1, Area.y1);
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
            point4_t *Clean = new point4_t{ gRenderingresolution.x, gRenderingresolution.y, 0, 0 };
            point4_t *Clipped;

            // Exchange the clipped area with a clean one.
            do { Clipped = Clippingarea.load(); }
            while (!Clippingarea.compare_exchange_strong(Clipped, Clean));

            /*
                Do rendering.
            */
        }
    }
}
#else
    #error Non-windows abstraction is not implemented (yet!)
#endif
