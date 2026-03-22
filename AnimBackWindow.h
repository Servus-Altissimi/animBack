#pragma once
#include "FrameLoader.h"
#include "ScreensaverRenderer.h"
#include <Button.h>
#include <MenuField.h>
#include <MessageRunner.h>
#include <PopUpMenu.h>
#include <Slider.h>
#include <StringView.h>
#include <TextControl.h>
#include <Window.h>
#include <vector>

// Message-codes shared with DeskbarApplet
const uint32 MSG_LOAD_FRAMES  = 'SETF';
const uint32 MSG_CLEAR_BG     = 'CLRB';
const uint32 MSG_START        = 'STAR';
const uint32 MSG_STOP         = 'STOP';
const uint32 MSG_NEXT_FRAME   = 'FRAM';
const uint32 MSG_SPEED_CHANGE = 'SPED';
const uint32 MSG_USE_SAVER    = 'SAVP';

class AnimBackWindow : public BWindow {
public:
    AnimBackWindow();

    void MessageReceived(BMessage* msg) override;
    bool QuitRequested() override;

    void StartAnimation();
    void StopAnimation();
    bool IsPlaying() const { return fPlaying; }

private:
    BTextControl* fPathInput;
    BButton*      fLoadButton;
    BPopUpMenu*   fSaverPopUp;
    BMenuField*   fSaverMenu;
    BButton*      fUseSaverButton;
    BSlider*      fFpsSlider;
    BButton*      fStartButton;
    BButton*      fStopButton;
    BButton*      fClearButton;
    BStringView*  fStatusLabel;

    std::vector<FrameInfo> fFrames;
    int32                  fCurrentFrame;
    BMessageRunner*        fRunner;
    bool                   fPlaying;
    bool                   fSaverMode;
    ScreensaverRenderer    fRenderer;

    static const char* kSaverTmp;

    void NextFrame();
    void UpdateControls();
    void Status(const char* msg);
    void PopulateSaverMenu();

    bigtime_t Delay() const { return 1000000 / fFpsSlider->Value(); }
};
