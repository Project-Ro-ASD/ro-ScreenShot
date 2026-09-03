# Building Ro-ScreenShot from Source

This document provides instructions for compiling, testing, and developing **Ro-ScreenShot** on Fedora 44+ and other modern Linux distributions.

---

## 📋 1. Prerequisites

### Toolchain Requirements
- **C++ Compiler:** GCC 13+ or Clang 16+ supporting **C++20** standard.
- **Build System:** CMake 3.22+ and Ninja (recommended).
- **Qt Framework:** Qt 6.5+ (Qt 6.11 recommended) with modules:
  - `qt6-qtbase-devel` (Core, Gui, DBus, Test)
  - `qt6-qtdeclarative-devel` (Quick, Qml)
  - `qt6-qtwayland-devel` (Wayland client integration)
- **Wayland portal runtime:**
  - `xdg-desktop-portal`
  - `xdg-desktop-portal-kde` on KDE Plasma (or the matching portal backend for
    the active desktop environment)

### Fedora 44+ Installation Command:
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel \
    qt6-qtwayland-devel \
    xdg-desktop-portal \
    xdg-desktop-portal-kde \
    desktop-file-utils \
    clang-tools-extra
```

---

## 🛠️ 2. Build Instructions

### Quick Build (Ninja):
```bash
# Clone the repository
git clone https://github.com/Project-Ro-ASD/Ro-ScreenShot.git ro-ScreenShot
cd ro-ScreenShot

# Configure with CMake
cmake -S . -B build-local -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON

# Compile binary
cmake --build build-local --parallel

# Run application
./build-local/ro-screenshot
```

---

## 🧪 3. Running Unit Tests

Ro-ScreenShot includes a CTest test suite covering the core modules:

```bash
# Run all tests via CTest
QT_QPA_PLATFORM=offscreen \
ctest --test-dir build-local --output-on-failure
```

Expected output:
```
100% tests passed, 0 tests failed out of 4
```

---

## 🧹 4. CMake Cache and Path Troubleshooting

Linux paths are case-sensitive. `Ro-ScreenShot` and `ro-ScreenShot` are
different paths. A build directory remembers the exact source path and CMake
generator used when it was created. Reusing it after renaming or moving the
repository produces errors such as:

```text
The current CMakeCache.txt directory is different than the directory where CMakeCache.txt was created.
The source does not match the source used to generate cache.
```

Keep the repository name consistent and configure a fresh, separate build
directory without deleting the old one:

```bash
cd ~/Desktop/Ro-ASD/ro-ScreenShot
cmake -S . -B build-local -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON
cmake --build build-local --parallel
```

Do not write `\ \` at the end of a shell line. A multiline command uses one
backslash as the final character of each continued line. The equivalent
single-line command is:

```bash
cmake -S . -B build-local -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
```

To intentionally reuse `build` with CMake 3.24 or newer, regenerate its cache:

```bash
cmake --fresh -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON
```

---

## 🎨 5. Code Formatting

The codebase enforces strict LLVM C++20 formatting:

```bash
# Reformat all C++ source and header files
find src tests -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +
```
