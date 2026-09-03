# Contributing to Ro-ScreenShot

Thank you for your interest in contributing to **Ro-ScreenShot**!

## Development Guidelines

1. **C++ Standard:** All code must conform to modern **C++20**.
2. **Formatting:** Follow `.clang-format` (LLVM-based style). Run `find src tests -type f \( -name "*.cpp" -o -name "*.hpp" \) -exec clang-format -i {} +` before committing.
3. **Tests:** All core changes must include unit tests in `tests/` and pass with 100% green via `QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure`.
4. **Commits:** Use conventional commits (`feat:`, `fix:`, `chore:`, `docs:`, `test:`).

## Submitting Pull Requests

1. Fork the repository and create a branch from `main`.
2. Implement your feature or bug fix with tests.
3. Verify that CI checks and tests pass locally.
4. Open a Pull Request referencing relevant issues.
