# animBack
**Animated desktop wallpapers and live screensaver rendering wallpapers for Haiku.**

[![License](https://img.shields.io/badge/license-MIT-green?style=for-the-badge)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Haiku-3399ff?style=for-the-badge)](https://www.haiku-os.org/)

---

Turns a folder of numbered images or any screensaver add-on into a lively desktop background.

## Features

- Load a folder of numbered frames and play them as a wallpaper animation
- Render any installed Haiku screensaver add-on directly to the desktop
- Adjustable playback speed from 1 to 30 FPS
- Average colour erase matching to suppress Tracker's blank-before-draw-flash
- Simple GUI
- CLI mode for autostart

## Requirements

- [Haiku](https://www.haiku-os.org/) (Tested only on x86_64 Stable)
- `libbe`, `libtracker`, `libtranslation`, `libscreensaver` (all included in a standard Haiku install)
- `g++` with C++17 support

## Build

```sh
make
```

For a quick install directly to `~/config/non-packaged/apps/animBack`:

```sh
make install
```

To produce a distributable `.hpkg`:

```sh
make package
```

## Use

Launch the GUI:

```sh
animback
```

Run from the terminal:

```sh
animback --animate /path/to/frames [fps]
animback --screensaver "/boot/system/add-ons/Screen Savers/screensaver" [fps]
```

Clear the desktop background:

```bash
animback --clear
```

### Image Frames

Frames must be image files named with integers only: `1.png`, `2.png`, `3.png`, and so on. Supported formats are anything the Haiku Translation Kit can decode: PNG, JPEG, BMP, GIF, TIFF. Frames are automatically scaled to screen resolution on load.
Place them in a folder, point animBack at it, and hit `Start`.

### Screensaver Mode

Any screensaver add-on installed under `/boot/system/add-ons/Screen Savers` or `~/config/add-ons/Screen Savers` will appear in the Screensaver tab. Select one and click **Use This Screensaver** to render it frame-by-frame to the desktop.
