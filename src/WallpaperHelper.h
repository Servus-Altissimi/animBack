#pragma once
#include <Bitmap.h>
#include <GraphicsDefs.h>
#include <SupportDefs.h>

namespace Wallpaper {
    // writes the background attr + sets the desktop erase color.
    // matching the erase color to the frame's average hides the
    // Tracker blank-before-draw flash that causes flicker.
    status_t Set(const char* imagePath, rgb_color bgColor);

    void Clear(); // blank image + refresh
    void Refresh(); // poke Tracker to redraw

    // Encode bitmap to PNG at path, does not free the bitmap.
    status_t SaveAsPng(BBitmap* bm, const char* path);
}
