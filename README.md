# ro-ScreenShot

A modern, lightning-fast screen capture, annotation, and library management tool engineered for Linux desktop environments with native Wayland and X11 support. Built using C++20 and Qt 6.

[![Build and Test](https://github.com/Project-Ro-ASD/Ro-ScreenShot/actions/workflows/build-test.yml/badge.svg)](https://github.com/Project-Ro-ASD/Ro-ScreenShot/actions)
[![License: GPL-3.0-or-later](https://img.shields.io/badge/License-GPL--3.0--or--later-blue.svg)](LICENSE)

---

## Overview

**ro-ScreenShot** is a next-generation screen capture, annotation, and library management tool engineered specifically for modern Linux desktop environments (Fedora 44+, KDE Plasma 6, GNOME, and Wayland compositors). It combines a zero-latency sniper capture engine with a comprehensive desktop library and flexible automation workflows.

### Core Capabilities

- **🎯 Interactive Sniper Overlay:** Fullscreen frozen-frame capture with sub-pixel rectangle selection, crosshair alignment guides, and a 2x–16x pixel loupe (magnifier) with live RGB/HEX color sampling (`C` to copy).
- **🖼️ Built-in Gallery & Library:** Integrated QML grid view to browse, filter, search, preview, copy, and manage historical screenshots with instant thumbnail caching.
- **⚡ Automated Workflows:** Configurable post-capture actions including auto-copying to system clipboard, auto-saving to formatted disk paths (`Ro-Shot_%Y-%m-%d_%H-%M-%S.png`), month-based subfolder grouping, and floating toast cards.
- **🔔 Floating Action Toast:** Non-intrusive 4-second thumbnail popup in the corner of your screen for rapid drag-and-drop or one-click copying.
- **🔌 D-Bus Single-Instance & CLI Service:** Standalone background D-Bus interface (`org.ro_asd.ScreenShot`) allowing instant binding to global shortcuts, terminal automation, or window manager scripts.

---

## CLI & Shortcut Quick Reference

When launched without arguments, `ro-screenshot` opens the graphical **Hub & Library**. Use CLI switches or desktop shortcuts for quick execution:

| Action | CLI Command | Suggested Shortcut | Description |
| :--- | :--- | :--- | :--- |
| **Region (Sniper)** | `ro-screenshot --region` (`-r`) | `Shift + PrintScreen` | Freezes display and opens interactive selection loupe |
| **Fullscreen** | `ro-screenshot --fullscreen` (`-f`) | `PrintScreen` | Captures all connected monitors instantly |
| **Active Window** | `ro-screenshot --window` (`-w`) | `Alt + PrintScreen` | Captures current focused window/screen |
| **Delayed Capture** | `ro-screenshot --delay <sec>` (`-d`) | `Ctrl + PrintScreen` | Waits `N` seconds before capture (great for context menus) |
| **Open Library** | `ro-screenshot --library` (`-l`) | `Super + Shift + G` | Opens screenshot gallery and management view |
| **Open Settings** | `ro-screenshot --settings` (`-s`) | — | Opens preferences and workflow configuration |

On KDE Plasma, the installed desktop actions publish the listed PrintScreen
shortcuts. If Spectacle already owns one, reassign that binding under **System
Settings -> Keyboard -> Shortcuts**; KDE permits only one owner per global
shortcut.

### Controls during Sniper Mode:
- **`Mouse Drag`**: Draw selection box.
- **`Enter / Return` / `Double Click`**: Confirm and process capture.
- **`Arrow Keys`**: Move the selection by one pixel.
- **`Shift + Arrow Keys`**: Resize the selection by one pixel.
- **`C`**: Copy hovered pixel HEX color code to clipboard.
- **`Escape`**: Cancel capture.

---

## Build and Run

Linux paths are case-sensitive. Clone into and consistently use the exact
`ro-ScreenShot` directory name:

```bash
git clone https://github.com/Project-Ro-ASD/Ro-ScreenShot.git ro-ScreenShot
cd ro-ScreenShot

cmake -S . -B build-local -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON
cmake --build build-local --parallel
./build-local/ro-screenshot
```

See the [build guide](docs/BUILD.md) for testing and CMake cache troubleshooting.

---

## Documentation

Comprehensive guides for users, maintainers, and developers:

- 📦 **[Installation & System Integration](docs/INSTALL.md):** Binary installation, desktop integration, and global shortcut setup.
- 🛠️ **[Build & Testing Guide](docs/BUILD.md):** Compiling from source on Fedora 44+, CMake parameters, running the CTest test suite.
- 🏗️ **[Architecture & D-Bus Specification](docs/ARCHITECTURE.md):** Core architecture, `CaptureEngine`, `LibraryManager`, memory caching, and D-Bus IPC methods.
- 🤝 **[Contributing Guidelines](CONTRIBUTING.md):** Development workflows, coding standards, and PR requirements.
- 🔒 **[Security Policy](SECURITY.md):** Reporting security issues and safety boundaries.

---

## License

Ro-ScreenShot is open-source software licensed under the **GNU General Public License v3.0 or later** (GPL-3.0-or-later). See [LICENSE](LICENSE) for details.
