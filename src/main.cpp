//  _______  __    _  ___   __   __  _______  _______  _______  ___   _ 
// |   _   ||  |  | ||   | |  |_|  ||  _    ||   _   ||       ||   | | |
// |  |_|  ||   |_| ||   | |       || |_|   ||  |_|  ||       ||   |_| |
// |       ||       ||   | |       ||       ||       ||       ||      _|
// |       ||  _    ||   | |       ||  _   | |       ||      _||     |_ 
// |   _   || | |   ||   | | ||_|| || |_|   ||   _   ||     |_ |    _  |
// |__| |__||_|  |__||___| |_|   |_||_______||__| |__||_______||___| |_|

// Copyright 2026 Servus Altissimi (Pseudonym)

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "AnimBackWindow.h"
#include "FrameLoader.h"
#include "ScreensaverRenderer.h"
#include "WallpaperHelper.h"
#include <Application.h>
#include <algorithm>
#include <cstdio>
#include <cstring>

class AnimBackApp : public BApplication {
public:
    AnimBackApp() : BApplication("application/x-vnd.cell-animBack") {}
    void ReadyToRun() override {
        (new AnimBackWindow())->Show();
    }
};

static void PrintUsage(const char* prog) {
    printf("Usage: %s [options]\n\n", prog);
    printf("  (no args)                       open the GUI\n");
    printf("  --animate <folder> [fps]        run image animation headlessly\n");
    printf("  --screensaver <addon> [fps]     run a screensaver add-on headlessly\n");
    printf("  --clear                         clear the desktop background\n");
    printf("  --help                          this message\n");
}

static void RunCli(const char* folder, int fps) {
    printf("Preparing frames…\n");

    std::vector<FrameInfo> frames;
    if (!PrepareFrames(folder, frames)) {
        fprintf(stderr, "No numbered frames found in '%s'\n", folder);
        return;
    }

    bigtime_t delay = 1000000 / fps;
    int32     cur   = 0;
    printf("%zu frame(s) at %d FPS — Ctrl+C to stop\n", frames.size(), fps);

    for (;;) {
        const FrameInfo& fi = frames[cur];
        if (Wallpaper::Set(fi.path.c_str(), fi.avg) == B_OK) {
            Wallpaper::Refresh();
            printf("\rFrame %d / %zu  ", cur + 1, frames.size());
            fflush(stdout);
        }
        cur = (cur + 1) % (int32)frames.size();
        snooze(delay);
    }
}

static void RunScreensaverCli(const char* addonPath, int fps) {
    ScreensaverRenderer renderer;
    if (renderer.Load(addonPath) != B_OK) {
        fprintf(stderr, "Failed to load screensaver add-on '%s'\n", addonPath);
        return;
    }

    static const char* kTmp = "/tmp/animback_saver.png";
    bigtime_t delay = 1000000 / fps;
    printf("Screensaver '%s' at %d FPS, Ctrl+C to stop\n", addonPath, fps);

    for (;;) {
        if (renderer.Tick(kTmp) != B_OK)
            fprintf(stderr, "Tick failed\n");
        snooze(delay);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        AnimBackApp().Run();
        return 0;
    }

    const char* arg = argv[1];

    if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) {
        PrintUsage(argv[0]);
        return 0;
    }

    if (!strcmp(arg, "--animate")) {
        if (argc < 3) { fprintf(stderr, "path required\n"); return 1; }
        int fps = 10;
        if (argc >= 4) fps = std::max(1, std::min(30, atoi(argv[3])));
        new BApplication("application/x-vnd.cell-animBack");
        RunCli(argv[2], fps);
        return 0;
    }

    if (!strcmp(arg, "--screensaver")) {
        if (argc < 3) { fprintf(stderr, "add-on path required\n"); return 1; }
        int fps = 10;
        if (argc >= 4) fps = std::max(1, std::min(30, atoi(argv[3])));
        new BApplication("application/x-vnd.cell-animBack");
        RunScreensaverCli(argv[2], fps);
        return 0;
    }

    if (!strcmp(arg, "--clear")) {
        new BApplication("application/x-vnd.cell-animBack");
        Wallpaper::Clear();
        printf("Cleared.\n");
        return 0;
    }

    fprintf(stderr, "Unknown option '%s', try --help\n", arg);
    return 1;
}
