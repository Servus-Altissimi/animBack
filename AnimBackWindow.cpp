#include "AnimBackWindow.h"
#include "WallpaperHelper.h"
#include <Entry.h>
#include <LayoutBuilder.h>
#include <TabView.h>
#include <cstdio>

const char* AnimBackWindow::kSaverTmp = "/tmp/animback_saver.png";

AnimBackWindow::AnimBackWindow()
    : BWindow(BRect(100, 100, 700, 380), "animBack",
              B_TITLED_WINDOW,
              B_QUIT_ON_WINDOW_CLOSE | B_AUTO_UPDATE_SIZE_LIMITS),
      fCurrentFrame(0), fRunner(nullptr), fPlaying(false), fSaverMode(false)
{
    // Images 
    fPathInput  = new BTextControl("path", "Folder:", "", nullptr);
    fPathInput->SetDivider(50);
    fLoadButton = new BButton("load", "Load Frames", new BMessage(MSG_LOAD_FRAMES));

    BView* imgTab = BLayoutBuilder::Group<>(B_VERTICAL, 8)
        .SetInsets(10)
        .AddGroup(B_HORIZONTAL, 8)
            .Add(fPathInput)
            .Add(fLoadButton)
        .End()
        .Add(new BStringView("hint", "Frames must be numbered: 1.png, 2.png, 3.png …"))
        .AddGlue()
        .View();
    imgTab->SetName("Image Folder");

    // Screensaver 
    fSaverPopUp     = new BPopUpMenu("(select screensaver)");
    fSaverMenu      = new BMenuField("saverMenu", "Screensaver:", fSaverPopUp);
    fUseSaverButton = new BButton("useSaver", "Use This Screensaver",
                                  new BMessage(MSG_USE_SAVER));
    PopulateSaverMenu();

    BView* saverTab = BLayoutBuilder::Group<>(B_VERTICAL, 8)
        .SetInsets(10)
        .Add(fSaverMenu)
        .Add(fUseSaverButton)
        .AddGlue()
        .View();
    saverTab->SetName("Screensaver");

    BTabView* tabs = new BTabView("tabs", B_WIDTH_FROM_LABEL);
    tabs->AddTab(imgTab);
    tabs->AddTab(saverTab);

    // Shared 
    fFpsSlider = new BSlider("fps", "Speed (FPS):", new BMessage(MSG_SPEED_CHANGE),
                             1, 30, B_HORIZONTAL);
    fFpsSlider->SetValue(10);
    fFpsSlider->SetHashMarks(B_HASH_MARKS_BOTTOM);
    fFpsSlider->SetHashMarkCount(10);
    fFpsSlider->SetLimitLabels("1", "30");
    fFpsSlider->SetTarget(this);

    fStartButton = new BButton("start", "▶  Start",  new BMessage(MSG_START));
    fStopButton  = new BButton("stop",  "■  Stop",   new BMessage(MSG_STOP));
    fClearButton = new BButton("clear", "Clear BG",  new BMessage(MSG_CLEAR_BG));
    fStatusLabel = new BStringView("status",
        "Load image frames or pick a screensaver, then press ▶ Start.");

    BLayoutBuilder::Group<>(this, B_VERTICAL, 8)
        .SetInsets(10)
        .Add(tabs)
        .Add(fFpsSlider)
        .AddGroup(B_HORIZONTAL, 8)
            .Add(fStartButton)
            .Add(fStopButton)
            .Add(fClearButton)
            .AddGlue()
        .End()
        .Add(fStatusLabel);

    UpdateControls();
}

bool AnimBackWindow::QuitRequested() {
    StopAnimation();
    return true;
}

void AnimBackWindow::MessageReceived(BMessage* msg) {
    switch (msg->what) {
        case MSG_LOAD_FRAMES: {
            const char* path = fPathInput->Text();
            if (!path || !BEntry(path).IsDirectory()) {
                Status("Invalid directory.");
                break;
            }
            Status("Loading frames…");
            UpdateIfNeeded(); // flush status label before blocking
            if (!PrepareFrames(path, fFrames)) {
                Status("No numbered frames found (1.png, 2.png, etc.)");
                break;
            }
            fSaverMode = false;
            fRenderer.Unload();
            char buf[128];
            snprintf(buf, sizeof(buf), "%zu frame(s) loaded!",
                     fFrames.size());
            Status(buf);
            UpdateControls();
            break;
        }

        case MSG_USE_SAVER: {
            BMenuItem* item = fSaverPopUp->FindMarked();
            if (!item) { Status("Pick a screensaver first."); break; }
            const char* p = nullptr;
            if (!item->Message() || item->Message()->FindString("path", &p) != B_OK) {
                Status("Can't find screensaver path.");
                break;
            }
            if (fRenderer.Load(p) != B_OK) {
                Status("Failed to load screensaver add-on.");
                break;
            }
            fSaverMode = true;
            fFrames.clear();
            Status("Screensaver loaded!");
            UpdateControls();
            break;
        }

        case MSG_CLEAR_BG:
            StopAnimation();
            Wallpaper::Clear();
            fPathInput->SetText("");
            Status("Cleared.");
            break;

        case MSG_START:       StartAnimation();  break;
        case MSG_STOP:        StopAnimation();   break;
        case MSG_NEXT_FRAME:  NextFrame();        break;

        case MSG_SPEED_CHANGE:
            if (fPlaying) { StopAnimation(); StartAnimation(); }
            break;

        default:
            BWindow::MessageReceived(msg);
    }
}

void AnimBackWindow::PopulateSaverMenu() {
    for (const auto& s : ScreensaverRenderer::List()) {
        BMessage* m = new BMessage(MSG_USE_SAVER);
        m->AddString("path", s.path.c_str());
        fSaverPopUp->AddItem(new BMenuItem(s.name.c_str(), m));
    }
    if (fSaverPopUp->CountItems() == 0)
        fSaverPopUp->AddItem(new BMenuItem("(none found)", nullptr));
}

void AnimBackWindow::StartAnimation() {
    bool ready = fSaverMode ? fRenderer.IsLoaded() : !fFrames.empty();
    if (!ready || fPlaying) return;

    fPlaying      = true;
    fCurrentFrame = 0;

    BMessage tick(MSG_NEXT_FRAME);
    fRunner = new BMessageRunner(this, &tick, Delay());

    UpdateControls();
    Status("Playing...");
    NextFrame(); // show first frame right away without waiting for the runner
}

void AnimBackWindow::StopAnimation() {
    if (!fPlaying) return;
    fPlaying = false;
    delete fRunner;
    fRunner = nullptr;
    UpdateControls();
    Status("Stopped.");
}

void AnimBackWindow::NextFrame() {
    if (fSaverMode) {
        fRenderer.Tick(kSaverTmp);
        return;
    }
    if (fFrames.empty()) return;

    const FrameInfo& fi = fFrames[fCurrentFrame];
    if (Wallpaper::Set(fi.path.c_str(), fi.avg) == B_OK) {
        Wallpaper::Refresh();
        char buf[64];
        snprintf(buf, sizeof(buf), "Frame %d / %zu",
                 fCurrentFrame + 1, fFrames.size());
        Status(buf);
    }
    fCurrentFrame = (fCurrentFrame + 1) % (int32)fFrames.size();
}

void AnimBackWindow::UpdateControls() {
    bool ready = fSaverMode ? fRenderer.IsLoaded() : !fFrames.empty();
    fStartButton->SetEnabled(ready && !fPlaying);
    fStopButton->SetEnabled(fPlaying);
}

void AnimBackWindow::Status(const char* msg) {
    fStatusLabel->SetText(msg);
}
