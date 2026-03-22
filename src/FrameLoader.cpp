#include "FrameLoader.h"
#include "WallpaperHelper.h"
#include <Directory.h>
#include <Entry.h>
#include <Path.h>
#include <Screen.h>
#include <TranslationUtils.h>
#include <View.h>
#include <algorithm>
#include <cstdio>

rgb_color AverageColor(BBitmap* bm) {
    if (!bm) return {0, 0, 0, 255};

    int    bpr = bm->BytesPerRow();
    uint8* px  = (uint8*)bm->Bits();

    // B, G, R, pad
    const int N = 16;
    uint32 r = 0, g = 0, b = 0;

    for (int yi = 0; yi < N; yi++) {
        int y = (int)(yi * bm->Bounds().Height() / (N - 1));
        for (int xi = 0; xi < N; xi++) {
            int    x = (int)(xi * bm->Bounds().Width() / (N - 1));
            uint8* p = px + y * bpr + x * 4;
            b += p[0]; g += p[1]; r += p[2];
        }
    }

    uint32 n = N * N;
    return {(uint8)(r/n), (uint8)(g/n), (uint8)(b/n), 255};
}

bool PrepareFrames(const char* folder, std::vector<FrameInfo>& out) {
    out.clear();

    BDirectory dir(folder);
    if (dir.InitCheck() != B_OK) return false;

    static const char* kExts[] = {"png","jpg","jpeg","bmp","gif","tiff", nullptr};
    std::vector<std::pair<int, std::string>> found;
    BEntry entry;

    while (dir.GetNextEntry(&entry) == B_OK) {
        if (!entry.IsFile()) continue;

        char name[B_FILE_NAME_LENGTH];
        entry.GetName(name);

        std::string fname(name);
        size_t dot = fname.rfind('.');
        if (dot == std::string::npos) continue;

        std::string stem = fname.substr(0, dot);
        std::string ext  = fname.substr(dot + 1);

        bool validExt = false;
        for (int i = 0; kExts[i]; i++)
            if (ext == kExts[i]) { validExt = true; break; }
        if (!validExt) continue;

        // Skip anything that isn't purely numeric
        bool numeric = !stem.empty();
        for (char c : stem)
            if (!isdigit((unsigned char)c)) { numeric = false; break; }
        if (!numeric) continue;

        BPath p; entry.GetPath(&p);
        found.push_back({std::stoi(stem), p.Path()});
    }

    if (found.empty()) return false;
    std::sort(found.begin(), found.end());

    BRect screen = BScreen().Frame().OffsetToCopy(0, 0);
    int   idx    = 0;

    for (auto& f : found) {
        BBitmap* src = BTranslationUtils::GetBitmap(f.second.c_str());
        if (!src) continue;

        // Scale to screen res so the per frame work is an attribute write
        BBitmap* scaled = new BBitmap(screen, B_RGB32, true);
        BView*   v      = new BView(screen, "", B_FOLLOW_NONE, B_WILL_DRAW);
        scaled->AddChild(v);
        scaled->Lock();
        v->DrawBitmap(src, src->Bounds(), screen);
        v->Sync();
        scaled->Unlock();
        scaled->RemoveChild(v);
        delete v;
        delete src;

        char tmp[64];
        snprintf(tmp, sizeof(tmp), "/tmp/animback_%d.png", idx++);

        if (Wallpaper::SaveAsPng(scaled, tmp) == B_OK)
            out.push_back({tmp, AverageColor(scaled)});

        delete scaled;
    }

    return !out.empty();
}
