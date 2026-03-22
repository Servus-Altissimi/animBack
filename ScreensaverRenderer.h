#pragma once
#include "FrameLoader.h"
#include <Bitmap.h>
#include <ScreenSaver.h>
#include <View.h>
#include <image.h>
#include <string>
#include <vector>

struct SaverInfo {
    std::string name;
    std::string path;
};

// Loads a screensaver add-on and renders it frame by frame into an
// offscreen bitmap, then pushes each frame to Tracker as a wallpaper.
class ScreensaverRenderer {
public:
    ScreensaverRenderer();
    ~ScreensaverRenderer();

    // Scan system + user screensaver dirs
    static std::vector<SaverInfo> List();

    status_t Load(const char* addonPath);
    void     Unload();

    // render one frame -> write to tmpPath -> set as wallpaper
    status_t Tick(const char* tmpPath);

    bool IsLoaded() const { return fSaver != nullptr; }

private:
    image_id      fAddon;
    BScreenSaver* fSaver;
    BBitmap*      fBitmap;  
    BView*        fView;
    int32         fFrame;
};
