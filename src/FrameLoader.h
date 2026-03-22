#pragma once
#include <Bitmap.h>
#include <GraphicsDefs.h>
#include <string>
#include <vector>

struct FrameInfo {
    std::string path; // pre-scaled PNG in /tmp
    rgb_color   avg; // average colour for desktop erase matching
};

// sample a 16x16 grid across a B_RGB32 bitmap and return the mean color
rgb_color AverageColor(BBitmap* bm);

// decode + scale all numerically named images in folder to screen res.
bool PrepareFrames(const char* folder, std::vector<FrameInfo>& out);
