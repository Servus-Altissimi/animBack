#include "ScreensaverRenderer.h"
#include "WallpaperHelper.h"
#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include <Screen.h>
#include <algorithm>

ScreensaverRenderer::ScreensaverRenderer()
    : fAddon(-1), fSaver(nullptr), fBitmap(nullptr), fView(nullptr), fFrame(0) {}

ScreensaverRenderer::~ScreensaverRenderer() {
    Unload();
}

std::vector<SaverInfo> ScreensaverRenderer::List() {
    std::vector<SaverInfo> out;
    const char* dirs[] = {
        "/boot/system/add-ons/Screen Savers",
        "/boot/home/config/add-ons/Screen Savers",
        nullptr
    };

    for (int i = 0; dirs[i]; i++) {
        BDirectory dir(dirs[i]);
        if (dir.InitCheck() != B_OK) continue;
        BEntry e;
        while (dir.GetNextEntry(&e) == B_OK) {
            if (!e.IsFile()) continue;
            BPath p; e.GetPath(&p);
            char name[B_FILE_NAME_LENGTH]; e.GetName(name);
            out.push_back({name, p.Path()});
        }
    }

    std::sort(out.begin(), out.end(),
        [](const SaverInfo& a, const SaverInfo& b) { return a.name < b.name; });
    return out;
}

status_t ScreensaverRenderer::Load(const char* path) {
    Unload();

    fAddon = load_add_on(path);
    if (fAddon < 0) return fAddon;

    using Fn = BScreenSaver*(*)(BMessage*, image_id);
    Fn fn = nullptr;
    if (get_image_symbol(fAddon, "instantiate_screen_saver",
                         B_SYMBOL_TYPE_TEXT, (void**)&fn) != B_OK) {
        unload_add_on(fAddon);
        fAddon = -1;
        return B_ERROR;
    }

    BMessage cfg;
    fSaver = fn(&cfg, fAddon);
    if (!fSaver) {
        unload_add_on(fAddon);
        fAddon = -1;
        return B_ERROR;
    }

    BRect bounds = BScreen().Frame().OffsetToCopy(0, 0);
    fBitmap = new BBitmap(bounds, B_RGB32, true);
    if (fBitmap->InitCheck() != B_OK) { Unload(); return B_ERROR; }

    fView = new BView(bounds, "sr", B_FOLLOW_NONE, B_WILL_DRAW);
    fBitmap->AddChild(fView);

    fBitmap->Lock();
    status_t s = fSaver->StartSaver(fView, false);
    fBitmap->Unlock();

    if (s != B_OK) { Unload(); return s; }
    return B_OK;
}

void ScreensaverRenderer::Unload() {
    if (fSaver) {
        if (fBitmap && fBitmap->Lock()) {
            fSaver->StopSaver();
            fBitmap->Unlock();
        }
        delete fSaver;
        fSaver = nullptr;
    }
    
    if (fBitmap) {
        delete fBitmap;
        fBitmap = nullptr;
        fView   = nullptr;
    }

    if (fAddon >= 0) {
        unload_add_on(fAddon);
        fAddon = -1;
    }
    fFrame = 0;
}

status_t ScreensaverRenderer::Tick(const char* tmpPath) {
    if (!fSaver || !fBitmap) return B_NO_INIT;

    fBitmap->Lock();
    fView->PushState();
    fSaver->Draw(fView, fFrame++);
    fView->Sync();
    fView->PopState();
    fBitmap->Unlock();

    rgb_color avg = AverageColor(fBitmap);

    status_t s = Wallpaper::SaveAsPng(fBitmap, tmpPath);
    if (s != B_OK) return s;

    s = Wallpaper::Set(tmpPath, avg);
    if (s == B_OK) Wallpaper::Refresh();
    return s;
}
