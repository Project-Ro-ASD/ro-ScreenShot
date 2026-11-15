# Shared Ro-Theme integration

Application surfaces use the pinned Ro-Theme palette in `data/theme/` through
`RoPalette.qml`. Main hub, gallery, settings, transient capture/recording windows,
editor chrome and dialogs share the same semantic color roles. Annotation ink,
image-export presets and sampled pixel colors remain document content.

Run `python3 tools/sync-theme.py --check` to verify snapshot hashes and generated
palette synchronization; regenerate with `python3 tools/sync-theme.py` after an
intentional upstream snapshot update. Run `python3 tools/validate-theme.py` for
light/dark text, selection, semantic action, focus and control-border contrast.
Both checks run in CI.

Geometry is centralized in `RoTokens.qml` using the Ro-Theme-Docs radius and
spacing contract. Text inherits the desktop font family; code/pixel metadata keep
a monospace font. Symbolic icons are recolored for the active surface. Custom
controls use Material by default so their backgrounds and content are supported
on all platforms; an explicit `QT_QUICK_CONTROLS_STYLE` remains respected.

`RoMotionScale.qml` applies the KDE `AnimationDurationFactor` read at startup from
`kdeglobals` using Qt configuration search precedence. Zero disables motion;
missing/invalid values use one. Restart the app after changing this system setting.
Tooltip delays, recording timers and other functional timing remain independent.

The `test_theme` QML suite covers live theme changes, selected foreground colors,
keyboard activation/focus, disabled and resized controls, loading/error states,
and disabled/scaled motion. Run the complete suite with:

```sh
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure
```

Desktop screenshot permission and compositor-specific capture behavior require
Linux/Wayland/X11 qualification; a macOS visual check does not establish that.
