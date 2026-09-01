# Ro-ScreenShot

**Modern, Wayland-Native Screen Capture & Gallery Suite for Ro-ASD and Linux.**

[![License: GPL-3.0-or-later](https://img.shields.io/badge/License-GPL--3.0--or--later-blue.svg)](LICENSE)
[![Platform: Ro-ASD / Linux](https://img.shields.io/badge/Platform-Ro--ASD%20%7C%20Fedora%2044%2B%20%7C%20Wayland-green.svg)](https://github.com/Project-Ro-ASD)
[![Framework: Qt6 & C++20](https://img.shields.io/badge/Framework-Qt%206.11%20%7C%20C%2B%2B20-blue.svg)](https://qt.io)
[![IPC: D-Bus Native](https://img.shields.io/badge/IPC-org.ro__asd.ScreenShot-orange.svg)](docs/ARCHITECTURE.md)
[![Ecosystem: Ro-ASD](https://img.shields.io/badge/Ecosystem-Project--Ro--ASD-red.svg)](https://github.com/Project-Ro-ASD)

---

## Overview

**Ro-ScreenShot** is a next-generation screen capture, annotation, and library management tool engineered specifically for modern Linux desktop environments (Fedora 44+, KDE Plasma 6, GNOME, and Wayland compositors). It combines a zero-latency sniper capture engine with a comprehensive desktop library and flexible automation workflows.

```
+-------------------------------------------------------------------------------+
|  📸 Ro-ScreenShot Hub                                      [—] [口] [X]       |
+-------------------------------------------------------------------------------+
|  [ 🔍 Search captures...       ]  [All | Today | Yesterday | This Week ▾]     |
+-------------------------------------------------------------------------------+
|  +--------------+  +--------------+  +--------------+  +--------------------+ |
|  |  [ Image ]   |  |  [ Image ]   |  |  [ Image ]   |  | ℹ️ DETAILS         | |
|  |              |  |              |  |              |  |                    | |
|  | 1920x1080    |  | 850x640      |  | 2560x1440    |  | File: Ro-Shot_01   | |
|  | Today, 14:20 |  | Today, 13:05 |  | Yest, 22:15  |  | Size: 1.4 MB       | |
|  | [📋][📂][🗑️] |  | [📋][📂][🗑️] |  | [📋][📂][🗑️] |  | Res: 2560 × 1440   | |
|  +--------------+  +--------------+  +--------------+  | [ Copy to Clp ]    | |
|                                                        | [ Open File   ]    | |
|                                                        | [ Show in Dir ]    | |
|                                                        | [ Delete      ]    | |
+--------------------------------------------------------+--------------------+ |
|  [ 🎯 Region / Sniper ]    [ 🖥️ Fullscreen ]    [ 🪟 Window ]    [ ⚙️ Settings ] |
+-------------------------------------------------------------------------------+
```

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

### Controls during Sniper Mode:
- **`Mouse Drag`**: Draw selection box.
- **`Enter / Return` / `Double Click`**: Confirm and process capture.
- **`C`**: Copy hovered pixel HEX color code to clipboard.
- **`Escape`**: Cancel capture.

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
