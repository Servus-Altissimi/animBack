CXX      := g++
CXXFLAGS := -O2 -Wall -std=c++17
LIBS     := -lbe -ltracker -ltranslation -lscreensaver

SRCS := src/main.cpp \
        src/WallpaperHelper.cpp \
        src/FrameLoader.cpp \
        src/ScreensaverRenderer.cpp \
        src/AnimBackWindow.cpp

PKG_NAME    := animback
PKG_VERSION := 1.0-1
PKG_ARCH    := x86_64
HPKG        := $(PKG_NAME)-$(PKG_VERSION)-$(PKG_ARCH).hpkg

INSTALL_DIR := /boot/home/config/non-packaged/apps/animBack
PKG_DIR     := /boot/home/config/packages
STAGE       := /tmp/$(PKG_NAME)-stage

.PHONY: all install package deploy clean

all: animback

animback: $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Quick install without packaging 
install: animback
	mkdir -p $(INSTALL_DIR)
	cp animback $(INSTALL_DIR)/animBack
	@echo "Installed to: $(INSTALL_DIR)/animBack"

# Package to .hpkg
package: animback
	rm -rf $(STAGE)
	mkdir -p $(STAGE)/apps/animBack
	cp animback   $(STAGE)/apps/animBack/animBack
	cp .PackageInfo $(STAGE)/.PackageInfo
	cd $(STAGE) && package create -C . $(CURDIR)/$(HPKG)
	@echo "Package: $(HPKG)"

clean:
	rm -rf animback $(HPKG) $(STAGE)
