# Building Ro-ScreenShot from Source

This document provides instructions for compiling, testing, and developing **Ro-ScreenShot** on Fedora 44+ and other modern Linux distributions.

---

## 📋 1. Prerequisites

### Toolchain Requirements
- **C++ Compiler:** GCC 13+ or Clang 16+ supporting **C++20** standard.
- **Build System:** CMake 3.22+ and Ninja (recommended).
- **Qt Framework:** Qt 6.5+ (Qt 6.11 recommended) with modules:
  - `qt6-qtbase-devel` (Core, Gui, DBus, Widgets, Test)
  - `qt6-qtdeclarative-devel` (Quick, Qml)
  - `qt6-qtwayland-devel` (Wayland client integration)

### Fedora 44+ Installation Command:
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    qt6-qtbase-devel \
    qt6-qtdeclarative-devel \
    qt6-qtwayland-devel \
    desktop-file-utils \
    clang-tools-extra
```

---

## 🛠️ 2. Build Instructions

### Quick Build (Ninja):
```bash
# Clone the repository
git clone https://github.com/Project-Ro-ASD/Ro-ScreenShot.git
cd Ro-ScreenShot

# Configure with CMake
cmake -B build -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON

# Compile binary
cmake --build build --parallel

# Run application
./build/ro-screenshot
```

---

## 🧪 3. Running Unit Tests

Ro-ScreenShot includes a CTest test suite covering the core modules:

```bash
# Run all tests via CTest
ctest --test-dir build --output-on-failure
```

Expected output:
```
100% tests passed, 0 tests failed out of 3
```

---

## 🎨 4. Code Formatting

The codebase enforces strict LLVM C++20 formatting:

```bash
# Reformat all C++ source and header files
find src tests -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +
```
