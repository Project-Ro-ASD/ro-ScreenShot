# Installation and System Integration Guide

This guide describes how to install **Ro-ScreenShot** system-wide and configure global desktop shortcuts.

---

## 📦 1. System Installation

### Install via CMake:
```bash
cmake --build build --target install
```

This installs:
- Binary: `/usr/local/bin/ro-screenshot`
- Desktop Launcher: `/usr/local/share/applications/org.ro_asd.ro_screenshot.desktop`

Update the desktop database:
```bash
sudo update-desktop-database
```

---

## ⌨️ 2. Global Shortcuts Configuration

### KDE Plasma 6 (System Settings -> Shortcuts):
1. Navigate to **System Settings** -> **Shortcuts** -> **Custom Commands**.
2. Add the following command bindings:
   - **Bölge Yakalama (Sniper):** `Shift + PrintScreen` -> `ro-screenshot --region`
   - **Tam Ekran Yakalama:** `PrintScreen` -> `ro-screenshot --fullscreen`
   - **Pencere Yakalama:** `Alt + PrintScreen` -> `ro-screenshot --window`
   - **Galeri & Hub:** `Super + Shift + G` -> `ro-screenshot --library`

### GNOME / Hyprland / Sway:
Add the following binds to your compositor configuration:

**Hyprland (`~/.config/hypr/hyprland.conf`):**
```ini
bind = , Print, exec, ro-screenshot --fullscreen
bind = SHIFT, Print, exec, ro-screenshot --region
bind = ALT, Print, exec, ro-screenshot --window
bind = SUPER SHIFT, G, exec, ro-screenshot --library
```

**Sway (`~/.config/sway/config`):**
```ini
bindsym Print exec ro-screenshot --fullscreen
bindsym Shift+Print exec ro-screenshot --region
bindsym Mod1+Print exec ro-screenshot --window
```
