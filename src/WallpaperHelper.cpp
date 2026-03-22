#include "WallpaperHelper.h"
#include <BitmapStream.h>
#include <File.h>
#include <FindDirectory.h>
#include <Messenger.h>
#include <Node.h>
#include <Path.h>
#include <Screen.h>
#include <TranslatorRoster.h>
#include <TranslatorFormats.h>
#include <be_apps/Tracker/Background.h>
#include <InterfaceDefs.h>

status_t Wallpaper::Set(const char* imagePath, rgb_color bgColor) {
    // Set erase color before writing attr: Tracker reads both in sequence
    BScreen().SetDesktopColor(bgColor);

    BPath deskPath;
    if (find_directory(B_DESKTOP_DIRECTORY, &deskPath) != B_OK)
        return B_ERROR;

    BNode node(deskPath.Path());
    if (node.InitCheck() != B_OK)
        return B_ERROR;

    BMessage msg;
    msg.AddInt32(B_BACKGROUND_WORKSPACES, (1 << count_workspaces()) - 1);
    msg.AddString(B_BACKGROUND_IMAGE, imagePath ? imagePath : "");
    msg.AddInt32(B_BACKGROUND_MODE, B_BACKGROUND_MODE_SCALED);
    msg.AddPoint(B_BACKGROUND_ORIGIN, BPoint(0, 0));
    msg.AddBool(B_BACKGROUND_ERASE_TEXT, true);

    ssize_t sz  = msg.FlattenedSize();
    char*   buf = new char[sz];
    status_t ret = B_ERROR;

    if (msg.Flatten(buf, sz) == B_OK &&
        node.WriteAttr(B_BACKGROUND_INFO, B_MESSAGE_TYPE, 0, buf, sz) == sz) {
        node.Sync();
        ret = B_OK;
    }

    delete[] buf;
    return ret;
}

void Wallpaper::Clear() {
    Set("", {0, 0, 0, 255});
    Refresh();
}

void Wallpaper::Refresh() {
    BMessenger("application/x-vnd.Be-TRAK").SendMessage(B_RESTORE_BACKGROUND_IMAGE);
}

status_t Wallpaper::SaveAsPng(BBitmap* bm, const char* path) {
    BFile file(path, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
    if (file.InitCheck() != B_OK) return B_ERROR;

    BBitmapStream stream(bm);
    status_t ret = BTranslatorRoster::Default()->Translate(
        &stream, nullptr, nullptr, &file, B_PNG_FORMAT);

    // Detach so the stream destructor doesn't free our bitmap
    BBitmap* tmp = nullptr;
    stream.DetachBitmap(&tmp);
    return ret;
}
