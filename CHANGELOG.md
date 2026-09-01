# Changelog

All notable changes to **Ro-ScreenShot** will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-09-02

### Added
- **Modern C++20 Core Architecture:** High-performance screen capture, settings management, and gallery model.
- **Wayland Native CaptureEngine:** Multi-monitor virtual desktop composition, sub-pixel region cropping, fullscreen and window capture.
- **Interactive Sniper Overlay (QML):** Frozen frame canvas, crosshair guidelines, dynamic dimension badge, and 2x–16x pixel loupe with live RGB/HEX color picker (`C` shortcut).
- **Library & Gallery Hub:** `QAbstractListModel` gallery view with live file system watcher, instant 320x240 thumbnail caching, date-based filtering (All, Today, Yesterday, This Week, This Month), and text search.
- **Workflow Automation:** Auto-copy to clipboard, dynamic template saving (`%Y`, `%m`, `%d`, `%H`, `%M`, `%S`), month-based subfolder grouping, and non-intrusive floating toast notifications.
- **D-Bus Single-Instance Service:** Standalone `org.ro_asd.ScreenShot` interface with CLI switches (`--region`, `--fullscreen`, `--window`, `--library`, `--settings`, `--delay`).
- **Unit Test Suite:** 3/3 CTest verification tests covering `SettingsManager`, `LibraryManager`, and `CaptureEngine`.
- **Packaging & Desktop Integration:** Desktop application entry, AppStream categories, and GitHub Actions CI workflow for Fedora 44.
