# Installation and System Integration Guide

This guide describes how to install **Ro-ScreenShot** system-wide and configure global desktop shortcuts.

---

## 📦 1. System Installation

### Install via CMake:
```bash
cmake -S . -B build-local -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF
cmake --build build-local --parallel
sudo cmake --install build-local
```

This installs:
- Binary: `/usr/local/bin/ro-screenshot`
- Desktop Launcher: `/usr/local/share/applications/org.ro_asd.ro_screenshot.desktop`

On Fedora KDE, the native Wayland capture backend requires the desktop portal
services (normally installed by Plasma):

```bash
sudo dnf install -y xdg-desktop-portal xdg-desktop-portal-kde
```

Update the desktop database:
```bash
sudo update-desktop-database /usr/local/share/applications
```

---

## ⌨️ 2. Global Shortcuts Configuration

### KDE Plasma 6 (System Settings -> Shortcuts):
The installed desktop file publishes these default KDE shortcuts directly:

- **Region:** `Shift + PrintScreen`
- **Fullscreen:** `PrintScreen`
- **Window:** `Alt + PrintScreen`
- **5-second delayed region:** `Ctrl + PrintScreen`

KDE Spectacle normally owns some PrintScreen combinations. KDE does not allow
two applications to own the same global shortcut. In **System Settings ->
Keyboard -> Shortcuts**, remove the conflicting binding from Spectacle and
assign it to **Ro-ScreenShot**. The in-application shortcuts work whenever the
Ro-ScreenShot window is focused, regardless of the global assignment.

If the application was installed or upgraded manually, refresh KDE's desktop
service cache:

```bash
kbuildsycoca6
```

Manual configuration:

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
