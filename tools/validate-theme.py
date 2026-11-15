#!/usr/bin/env python3
"""Check contrast for the shared application palette in both variants."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def luminance(color):
    channels = [int(color[i:i + 2], 16) / 255 for i in (1, 3, 5)]
    linear = [c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
              for c in channels]
    return sum(c * weight for c, weight in zip(linear, (0.2126, 0.7152, 0.0722)))


def contrast(first, second):
    values = sorted((luminance(first), luminance(second)))
    return (values[1] + 0.05) / (values[0] + 0.05)


for mode in ('light', 'dark'):
    palette = json.loads((ROOT / f'data/theme/colors.{mode}.json').read_text())
    pairs = [(text, background, 4.5)
             for text in ('text', 'textSecondary')
             for background in ('bg', 'surface', 'surfaceAlt')]
    pairs += [('selectionText', 'accent', 4.5),
              ('selectionText', 'accentHover', 4.5),
              ('bg', 'negative', 4.5), ('accent', 'surface', 3),
              ('controlBorder', 'surface', 3)]
    for foreground, background, minimum in pairs:
        ratio = contrast(palette[foreground], palette[background])
        assert ratio >= minimum, (mode, foreground, background, ratio, minimum)
    print(f'{mode}: {len(pairs)} contrast checks passed')
